#ifndef FEAUX_S_BENCHMARKING
#include <emscripten.h>
#endif

#include <list>
#include <queue>

#include "browser-api.h"
#include "decls.h"
#include "machine.h"
#include "os.h"
#include "process.h"
#include "utils.h"

#if FEAUX_S_BENCHMARKING
#include "benchmarks.h"
#endif

#ifndef FEAUX_S_BENCHMARKING
// clang-format off
// A sleep function that is non-blocking
EM_JS(void, jssleep, (int milis), { Asyncify.handleSleep(wakeUp => { setTimeout(wakeUp, milis); }); });
// clang-format on
#endif

#define PRINT_SIZE(type) cout << #type ": " << sizeof(type) << endl

// The kernel of our "OS"
int main() {
#if FEAUX_S_BENCHMARKING
	for (int strategy = SchedulingStrategy::FIFO; strategy <= SchedulingStrategy::MLF; strategy++) {
#endif

		initMachine(2, 1);
#if FEAUX_S_BENCHMARKING
		initOS(machine->numCores, (SchedulingStrategy)strategy);
#else
	initOS(machine->numCores, SchedulingStrategy::FIFO);
#endif

#if FEAUX_S_BENCHMARKING
		bool processesComing = true;
#endif

		while (true) {
			if (state->paused) {
				goto skip;	// Skip all the normal operations of the OS and just do a NOOP this tick
			}

			// Update our current time step
			state->time++;

#if FEAUX_S_BENCHMARKING
			processesComing = simulate();
#endif

			// If in RT mode, check RT jobs
			if (state->strategy == SchedulingStrategy::RT_FIFO || state->strategy == SchedulingStrategy::RT_LST ||
				state->strategy == SchedulingStrategy::RT_EDF) {
				for (RTJob* job : state->jobList) {
					if ((state->time - job->delay) % job->period == 0) {
						spawn(job->program.c_str(), state->time + job->deadline);
					}
				}
			}

			// Tick the CPUs and I/O devices
			for (uint8_t i = 0; i < machine->numCores; i++) machine->cores[i]->tick();
			for (uint8_t i = 0; i < machine->numIODevices; i++) machine->ioDevices[i]->tick();

			for (uint core = 0; core < machine->numCores; core++) {	 // For each core in our simulated device
				PCB* runningProcess = state->runningProcess[core];	 // The currently running process on this core

				state->stepAction[core] = StepAction::NOOP;	 // Initialize action to NOOP, update later

				if (machine->cores[core]->free()) {	 // If the core isn't running anything atm
					if (!state->pendingRequests
							 .empty()) {  // If there was an I/O request issued, but all the I/O devices at the time were busy at the time...
						bool deviceAvailable = false;
						for (uint i = 0; i < machine->numIODevices; i++) {
							if (!machine->ioDevices[i]->busy()) {
								deviceAvailable = true;
								break;
							}
						}

						if (deviceAvailable) {	// If there is now a device available, service that request
							state->stepAction[core] = StepAction::SERVICE_REQUEST;
						}
					}

					if (state->stepAction[core] == StepAction::NOOP) {	// If the core is not servicing an I/O request
						if (!state->interrupts.empty()) {
							state->stepAction[core] = StepAction::HANDLE_INTERRUPT;	 // handle an interrupt
						} else {
							switch (state->strategy) {
								case SchedulingStrategy::FIFO:
								case SchedulingStrategy::RT_FIFO:
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
								case SchedulingStrategy::RT_EDF:
									if (!state->edfReadyList.empty()) {
										state->stepAction[core] = StepAction::BEGIN_RUN;
										break;
									}
									break;
								case SchedulingStrategy::RT_LST:
									if (!state->lstReadyList.empty()) {
										state->stepAction[core] = StepAction::BEGIN_RUN;
										break;
									}
									break;
								default:
									cerr << "Debug: unrecognized scheduling strategy " << state->strategy << endl;
									break;
							}
						}
					}
				} else {													  // The CPU is currently running a process
					if (state->pendingSyscalls[core] != Syscall::SYS_NONE) {  // The currently running process issued a syscall
						state->stepAction[core] = StepAction::HANDLE_SYSCALL;
					} else if (state->strategy == SchedulingStrategy::MLF) {  // Might need to reschedule if using Multi-level Feedback scheduling (if a
																			  // process was just spawned)
						// Check whether there exists an available core
						bool coreAvailable = false;
						for (uint i = 0; i < machine->numCores; i++) {
							if (machine->cores[i]->free()) {
								coreAvailable = true;
								break;
							}
						}

						if (!coreAvailable) {  // if not, then the new process (if it exists) will pre-empt the process running on this core
							for (uint i = 0; i < runningProcess->level; i++) {
								if (!state->mlfLists[i].empty()) {
									state->stepAction[core] = StepAction::BEGIN_RUN;  // If a process was found on a higher priority level than the currently
																					  // running process, then pre-empt the process running on this core
									break;
								}
							}
						}

						if (state->stepAction[core] != StepAction::BEGIN_RUN) {	 // If no such process was found, then continue execution
							state->stepAction[core] = StepAction::CONTINUE_RUN;
						}
					} else if (state->strategy == SchedulingStrategy::RT_EDF && state->edfReadyList.top()->deadline != -1 &&
							   (runningProcess->deadline == -1 || state->edfReadyList.top()->deadline < runningProcess->deadline)) {
						// Reset state
						runningProcess->state = ready;
						runningProcess->processorTime++;

						// Save register state
						Registers regstate = machine->cores[core]->regstate();
						runningProcess->regstate = regstate;

						// Load preempting process (modeling 0 context-switching time; alternatively, resetting core to no process would model 1-tick
						// context-switching cost)
						PCB* preProc = schedule(core);
						state->edfReadyList.push(runningProcess);

						runningProcess = preProc;
						state->runningProcess[core] = preProc;
						machine->cores[core]->load(preProc->regstate);

						state->stepAction[core] = StepAction::CONTINUE_RUN;
					} else if (state->strategy == SchedulingStrategy::RT_LST && state->lstReadyList.top()->deadline != -1 &&
							   (runningProcess->deadline == -1 ||
								// very verbose way of writing slack time
								state->lstReadyList.top()->deadline -
										(state->time + (state->lstReadyList.top()->reqProcessorTime - state->lstReadyList.top()->processorTime + 1)) <
									runningProcess->deadline - (state->time + (runningProcess->reqProcessorTime - runningProcess->processorTime + 1)))) {
						// Reset state
						runningProcess->state = ready;
						runningProcess->processorTime++;

						// Save register state
						Registers regstate = machine->cores[core]->regstate();
						runningProcess->regstate = regstate;

						// Load preempting process (modeling 0 context-switching time; alternatively, resetting core to no process would model 1-tick
						// context-switching cost)
						PCB* preProc = schedule(core);
						state->lstReadyList.push(runningProcess);

						runningProcess = preProc;
						state->runningProcess[core] = preProc;
						machine->cores[core]->load(preProc->regstate);

						state->stepAction[core] = StepAction::CONTINUE_RUN;
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

									// Find the process for whom the I/O operation completed
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

							delete interrupt;  // Free the memory allocated for this interrupt (see machine.cpp#IODevice::tick)
						} else {
							cerr << "Debug, core " << core << ": trying to handle nonexistent interrupt" << endl;
							return 1;
						}
						break;
					}
					case StepAction::BEGIN_RUN:
						runningProcess = schedule(core);  // Pick a process to run

						if (runningProcess == nullptr) {
							cerr << "Debug, core " << core << ": Attempting to run a nonexistent process" << endl;
							return 1;
						}

						runningProcess->state = processing;					   // Mark the process as running
						state->runningProcess[core] = runningProcess;		   // Keep track of the process in the OS state
						machine->cores[core]->load(runningProcess->regstate);  // Load the process's registers into the CPU to execute the program
						break;
					case StepAction::CONTINUE_RUN:
						if (runningProcess != nullptr) {
							runningProcess->processorTime++;  // Tick the simulation times
							if (state->strategy == SchedulingStrategy::MLF) {
								runningProcess->processorTimeOnLevel++;	 // Tick the simulation times
							}

							if (state->strategy == SchedulingStrategy::MLF	// If we are using MLF scheduling
								&& runningProcess->level < NUM_LEVELS - 1	// If the current process is not on the lowest level
																			// (ie. the process does have a level time limit)
								&&
								runningProcess->processorTimeOnLevel > (0b10 << runningProcess->level)	// If the process has received the limit of CPU time
																										// (0b10 left-shifted by the level is a
																										// tricky way of doing 2^level)
							) {
								// Reset state
								runningProcess->state = ready;
								runningProcess->level++;
								runningProcess->processorTimeOnLevel = 0;

								// Save register state
								Registers regstate = machine->cores[core]->regstate();
								runningProcess->regstate = regstate;
								state->reentryList.push_back(runningProcess);

								// Clear CPU and running process entry
								state->runningProcess[core] = nullptr;
								machine->cores[core]->load(NOPROC);
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
									// Check whether there is a free I/O device to handle the request
									int freeDevice = -1;
									for (int i = 0; i < machine->numIODevices; i++) {
										if (!machine->ioDevices[i]->busy()) {
											freeDevice = i;
											break;
										}
									}

									// Mark the process as blocked
									runningProcess->state = blocked;
									if (freeDevice == -1) {	 // If there is no I/O device available
										runningProcess->regstate = machine->cores[core]->regstate();
										state->pendingRequests.push(IORequest{runningProcess->pid, (uint8_t)runningProcess->regstate.rdi});
									} else {
										if (state->pendingRequests.empty()) {  // If this is the only I/O request pending, just pass it to the I/O device
											runningProcess->regstate = machine->cores[core]->regstate();
											machine->ioDevices[freeDevice]->handle(IORequest{runningProcess->pid, (uint8_t)runningProcess->regstate.rdi});
										} else {
											// If there were other I/O requests made previously, save the register states and I/O request details
											runningProcess->regstate = machine->cores[core]->regstate();
											state->pendingRequests.push(IORequest{runningProcess->pid, (uint8_t)runningProcess->regstate.rdi});

											// Service the first I/O request to be submitted
											IORequest req = state->pendingRequests.front();
											state->pendingRequests.pop();
											machine->ioDevices[freeDevice]->handle(req);
										}
									}

									runningProcess->processorTime++;
									runningProcess = nullptr;
									state->runningProcess[core] = nullptr;
									machine->cores[core]->load(NOPROC);
									state->pendingSyscalls[core] = Syscall::SYS_NONE;
									break;
								}
								case Syscall::SYS_EXIT:
									// Mark processs as done and save final register state
									runningProcess->state = (runningProcess->deadline == -1 || state->time <= runningProcess->deadline) ? done : dead;
									runningProcess->doneTime = state->time;
									runningProcess->regstate = machine->cores[core]->regstate();

									runningProcess->processorTime++;
									runningProcess = nullptr;
									state->runningProcess[core] = nullptr;
									machine->cores[core]->load(NOPROC);
									state->pendingSyscalls[core] = Syscall::SYS_NONE;
									break;
								case Syscall::SYS_ALLOC: {
									uint size = machine->cores[core]->regstate().rdi, destRegister = machine->cores[core]->regstate().rsi;
									char* memory = new char[size];

									uint* dest = getRegister(machine->cores[core]->_registers, (Regs)destRegister);
									*dest = (uint)memory;
									machine->cores[core]->_registers.rax = size;

									runningProcess->processorTime++;
									state->pendingSyscalls[core] = Syscall::SYS_NONE;
									break;
								}
								case Syscall::SYS_FREE: {
									char* ptr = (char*)*getRegister(machine->cores[core]->_registers, (Regs)machine->cores[core]->regstate().rdi);

									delete[] ptr;
									machine->cores[core]->_registers.rax = 0;

									runningProcess->processorTime++;
									state->pendingSyscalls[core] = Syscall::SYS_NONE;
									break;
								}
							}
						} else {
							cerr << "Debug, core " << core << ": No running process... somehow" << endl;
							return 1;
						}
						break;
					case StepAction::SERVICE_REQUEST: {
						// Find the I/O device that is free
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

			// For all the processes that were unblocked during this step, insert them into the appropriate ready list
			for (auto it = state->reentryList.begin(); it != state->reentryList.end(); it++) {
				switch (state->strategy) {
					case SchedulingStrategy::FIFO:
					case SchedulingStrategy::RT_FIFO:
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
					case SchedulingStrategy::RT_EDF:
						state->edfReadyList.emplace(*it);
						break;
					case SchedulingStrategy::RT_LST:
						state->lstReadyList.emplace(*it);
						break;
				}
			}
			state->reentryList.clear();

#if FEAUX_S_BENCHMARKING
			if (!processesComing) {
				bool shouldBreak = true;

				for (auto it = state->processList.begin(); it != state->processList.end(); it++) {
					if ((*it)->state != done) {
						shouldBreak = false;
						break;
					}
				}

				if (shouldBreak) {
					break;
				}
			}
#endif

		skip:
#if FEAUX_S_BENCHMARKING
			void(0);
#else
		jssleep(machine->clockDelay);
#endif
		}

#if FEAUX_S_BENCHMARKING
		printStats();
		cleanupOS();
		cleanupMachine();
	}
#endif

	return 0;
}
