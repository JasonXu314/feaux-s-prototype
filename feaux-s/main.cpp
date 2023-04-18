#include <emscripten.h>

#include <list>
#include <queue>

#include "browser-api.h"
#include "decls.h"
#include "machine.h"
#include "os.h"
#include "process.h"
#include "utils.h"

// clang-format off
EM_JS(void, jssleep, (int milis), { Asyncify.handleSleep(wakeUp => { setTimeout(wakeUp, milis); }); });
// clang-format on

#define PRINT_SIZE(type) cout << #type ": " << sizeof(type) << endl

int main() {
	initMachine(2, 1);
	initOS(machine->numCores, SchedulingStrategy::FIFO);

	// keep running the loop until all processes have been added and have run to completion
	while (true) {
		if (state->paused) {
			goto skip;
		}

		// Update our current time step
		state->time++;

		for (uint8_t i = 0; i < machine->numCores; i++) machine->cores[i]->tick();
		for (uint8_t i = 0; i < machine->numIODevices; i++) machine->ioDevices[i]->tick();

		// If the processor is tied up running a process, then continue running it until it is done or blocks
		//    note: be sure to check for things that should happen as the process continues to run (io, completion...)
		// If the processor is free then you can choose the appropriate action to take, the choices (in order of precedence) are:
		//  - admit a new process if one is ready (i.e., take a 'newArrival' process and put them in the 'ready' state)
		//  - address an interrupt if there are any pending (i.e., update the state of a blocked process whose IO operation is complete)
		//  - start processing a ready process if there are any ready

		// init the stepAction, update below
		for (uint core = 0; core < machine->numCores; core++) {
			PCB* runningProcess = state->runningProcess[core];

			state->stepAction[core] = StepAction::NOOP;

			if (machine->cores[core]->free()) {
				if (!state->pendingRequests.empty()) {
					bool deviceAvailable = false;
					for (uint i = 0; i < machine->numIODevices; i++) {
						if (!machine->ioDevices[i]->busy()) {
							deviceAvailable = true;
							break;
						}
					}

					if (deviceAvailable) {
						state->stepAction[core] = StepAction::SERVICE_REQUEST;
					}
				}

				if (state->stepAction[core] == StepAction::NOOP) {
					if (!state->interrupts.empty()) {
						state->stepAction[core] = StepAction::HANDLE_INTERRUPT;	 // handle an interrupt
					} else {
						switch (state->strategy) {
							case SchedulingStrategy::FIFO:
								if (!state->fifoReadyList.empty()) {
									state->stepAction[core] = StepAction::BEGIN_RUN;  // start running a process
								}
								break;
							case SchedulingStrategy::SJF:
								if (!state->sjfReadyList.empty()) {
									state->stepAction[core] = StepAction::BEGIN_RUN;  // start running a process
								}
								break;
							case SchedulingStrategy::SRT:
								if (!state->srtReadyList.empty()) {
									state->stepAction[core] = StepAction::BEGIN_RUN;  // start running a process
								}
								break;
							case SchedulingStrategy::MLF:
								for (int i = 0; i < NUM_LEVELS; i++) {
									if (!state->mlfLists[i].empty()) {
										state->stepAction[core] = StepAction::BEGIN_RUN;  // start running a process
										break;
									}
								}
								break;
							default:
								cerr << "Debug: unrecognized scheduling strategy " << state->strategy << endl;
								break;
						}
					}
				}
			} else {
				if (state->pendingSyscalls[core] != Syscall::SYS_NONE) {
					state->stepAction[core] = StepAction::HANDLE_SYSCALL;
				} else if (state->strategy == SchedulingStrategy::MLF) {  // might need to reschedule if using Multi-level Feedback scheduling
					bool coreAvailable = false;
					for (uint i = 0; i < machine->numCores; i++) {
						if (machine->cores[i]->free()) {
							coreAvailable = true;
							break;
						}
					}

					if (!coreAvailable) {
						for (uint i = 0; i < runningProcess->level; i++) {
							if (!state->mlfLists[i].empty()) {
								state->stepAction[core] = StepAction::BEGIN_RUN;
								break;
							}
						}
					}

					if (state->stepAction[core] != StepAction::BEGIN_RUN) {
						state->stepAction[core] = StepAction::CONTINUE_RUN;
					}
				} else {
					state->stepAction[core] = StepAction::CONTINUE_RUN;	 // runnning process is still running
				}
			}

			switch (state->stepAction[core]) {
				case StepAction::HANDLE_INTERRUPT: {
					if (!state->interrupts.empty()) {
						Interrupt* interrupt = state->interrupts.front();
						state->interrupts.pop_front();

						switch (interrupt->type()) {
							case InterruptType::IO_COMPLETION: {
								IOInterrupt* ioInterrupt = (IOInterrupt*)interrupt;

								PCB* originProcess = nullptr;
								for (auto it = state->processList.begin(); it != state->processList.end(); it++) {
									if ((*it)->pid == ioInterrupt->pid()) {
										originProcess = *it;
									}
								}

								if (originProcess == nullptr) {
									cerr << "Debug, core " << core << ": unable to find origin process of IOEvent" << endl;
									return 1;
								} else {
									originProcess->state = ready;
									state->reentryList.push_back(originProcess);
								}
								break;
							}
							default:
								cerr << "Debug, core " << core << ": Unknown interrupt type " << interrupt->type() << endl;
								break;
						}

						delete interrupt;
					} else {
						cerr << "Debug, core " << core << ": trying to handle nonexistent interrupt" << endl;
						return 1;
					}
					break;
					case StepAction::BEGIN_RUN:
						runningProcess = schedule(core);

						if (runningProcess == nullptr) {
							cerr << "Debug, core " << core << ": Attempting to run a nonexistent process" << endl;
							return 1;
						}

						runningProcess->state = processing;
						state->runningProcess[core] = runningProcess;
						machine->cores[core]->load(runningProcess->regstate);
						break;
				}
				case StepAction::CONTINUE_RUN:
					if (runningProcess != nullptr) {
						runningProcess->processorTime++;
						if (state->strategy == SchedulingStrategy::MLF) {
							runningProcess->processorTimeOnLevel++;
						}

						if (state->strategy == SchedulingStrategy::MLF && runningProcess->level < NUM_LEVELS - 1 &&
							runningProcess->processorTimeOnLevel > (0b10 << runningProcess->level)) {
							runningProcess->state = ready;
							runningProcess->level++;
							runningProcess->processorTimeOnLevel = 0;
							state->reentryList.push_back(runningProcess);

							state->runningProcess[core] = nullptr;
							Registers regstate = machine->cores[core]->regstate();
							machine->cores[core]->load(NOPROC);
							runningProcess->regstate = regstate;
						}
					} else {
						cerr << "Debug, core " << core << ": trying to run a nonexistent process" << endl;
						return 1;
					}
					break;
				case StepAction::HANDLE_SYSCALL:
					if (runningProcess != nullptr) {
						switch (state->pendingSyscalls[core]) {
							case Syscall::SYS_NONE:
								cerr << "Debug, core " << core << ": handling nonexistent syscall" << endl;
								return 1;
							case Syscall::SYS_IO: {
								int freeDevice = -1;
								for (int i = 0; i < machine->numIODevices; i++) {
									if (!machine->ioDevices[i]->busy()) {
										freeDevice = i;
										break;
									}
								}

								runningProcess->state = blocked;
								if (freeDevice == -1) {
									runningProcess->regstate = machine->cores[core]->regstate();
									state->pendingRequests.push(IORequest{runningProcess->pid, (uint8_t)runningProcess->regstate.rdi});
								} else {
									if (state->pendingRequests.empty()) {
										runningProcess->regstate = machine->cores[core]->regstate();
										machine->ioDevices[freeDevice]->handle(IORequest{runningProcess->pid, (uint8_t)runningProcess->regstate.rdi});
									} else {
										runningProcess->regstate = machine->cores[core]->regstate();
										state->pendingRequests.push(IORequest{runningProcess->pid, (uint8_t)runningProcess->regstate.rdi});

										IORequest req = state->pendingRequests.front();
										state->pendingRequests.pop();
										machine->ioDevices[freeDevice]->handle(req);
									}
								}
								break;
							}
							case Syscall::SYS_EXIT:
								runningProcess->state = done;
								runningProcess->doneTime = state->time;
								break;
						}

						runningProcess->processorTime++;
						runningProcess = nullptr;
						state->runningProcess[core] = nullptr;
						machine->cores[core]->load(NOPROC);
						state->pendingSyscalls[core] = Syscall::SYS_NONE;
					} else {
						cerr << "Debug, core " << core << ": No running process... somehow" << endl;
						return 1;
					}
					break;
				case StepAction::SERVICE_REQUEST: {
					int freeDevice = -1;
					for (int i = 0; i < machine->numIODevices; i++) {
						if (!machine->ioDevices[i]->busy()) {
							freeDevice = i;
							break;
						}
					}

					if (freeDevice == -1) {
						cerr << "Debug, core " << core << ": attempting to service request, but no available device" << endl;
						return 1;
					} else {
						IORequest req = state->pendingRequests.front();
						state->pendingRequests.pop();
						machine->ioDevices[freeDevice]->handle(req);
					}
					break;
				}
				case StepAction::NOOP:
					break;
			}
		}

		for (auto it = state->reentryList.begin(); it != state->reentryList.end(); it++) {
			switch (state->strategy) {
				case SchedulingStrategy::FIFO:
					state->fifoReadyList.emplace(*it);
					break;
				case SchedulingStrategy::SJF:
					state->sjfReadyList.emplace(*it);
					break;
				case SchedulingStrategy::SRT:
					state->srtReadyList.emplace(*it);
					break;
				case SchedulingStrategy::MLF:
					state->mlfLists[(*it)->level].emplace(*it);
					break;
			}
		}
		state->reentryList.clear();

	skip:
		jssleep(machine->clockDelay);
	}

	return 0;
}
