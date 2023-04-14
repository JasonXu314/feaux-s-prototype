#include <emscripten.h>

#include <list>
#include <queue>

#include "decls.h"
#include "ioModule.h"
#include "process.h"
#include "utils.h"

// clang-format off
EM_JS(void, jssleep, (int milis), { Asyncify.handleSleep(wakeUp => { setTimeout(wakeUp, milis); }); });
// clang-format on

void initOS(uint numCores, SchedulingStrategy strategy);
void cleanupOS();

Process* schedule(uint core);

int main() {
	cout << sizeof(OSStateCompat) << endl;

	machineState = new MachineState{2, 500, new bool[2]{true, true}, new Process*[2]};

	initOS(machineState->numCores, SchedulingStrategy::FIFO);

	// keep running the loop until all processes have been added and have run to completion
	while (true) {
		if (state->paused) {
			goto skip;
		}

		// Update our current time step
		++(state->time);

		// update the status for any active IO requests
		state->ioModule->ioProcessing(state->time);

		// If the processor is tied up running a process, then continue running it until it is done or blocks
		//    note: be sure to check for things that should happen as the process continues to run (io, completion...)
		// If the processor is free then you can choose the appropriate action to take, the choices (in order of precedence) are:
		//  - admit a new process if one is ready (i.e., take a 'newArrival' process and put them in the 'ready' state)
		//  - address an interrupt if there are any pending (i.e., update the state of a blocked process whose IO operation is complete)
		//  - start processing a ready process if there are any ready

		// init the stepAction, update below
		for (uint core = 0; core < machineState->numCores; core++) {
			Process* runningProcess = machineState->runningProcess[core];

			state->stepAction[core] = StepAction::NOOP;

			if (machineState->available[core]) {
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
			} else {
				if (runningProcess->processorTime == runningProcess->reqProcessorTime) {
					state->stepAction[core] = StepAction::COMPLETE;	 // running process is finished
				} else if (!runningProcess->ioEvents.empty() && runningProcess->ioEvents.front().time == runningProcess->processorTime) {
					state->stepAction[core] = StepAction::IO_REQUEST;	  // running process issued an io request
				} else if (state->strategy == SchedulingStrategy::MLF) {  // might need to reschedule if using Multi-level Feedback scheduling
					for (uint i = 0; i < runningProcess->level; i++) {
						if (!state->mlfLists[i].empty()) {
							state->stepAction[core] = StepAction::BEGIN_RUN;
							break;
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
				case StepAction::HANDLE_INTERRUPT:
					if (!state->interrupts.empty()) {
						IOInterrupt interrupt = state->interrupts.front();
						state->interrupts.pop_front();

						Process* originProcess = nullptr;
						for (auto it = state->processList.begin(); it != state->processList.end(); it++) {
							if ((*it)->id == interrupt.procID) {
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
					runningProcess->processorTime++;
					machineState->available[core] = false;
					machineState->runningProcess[core] = runningProcess;
					break;
				case StepAction::CONTINUE_RUN:
					if (runningProcess != nullptr) {
						runningProcess->processorTime++;
						if (state->strategy == SchedulingStrategy::MLF) {
							runningProcess->processorTimeOnLevel++;
						}

						if (state->strategy == SchedulingStrategy::MLF && runningProcess->level < NUM_LEVELS - 1 &&
							runningProcess->processorTimeOnLevel >= (0b10 << runningProcess->level)) {
							runningProcess->state = ready;
							runningProcess->level++;
							runningProcess->processorTimeOnLevel = 0;
							state->reentryList.push_back(runningProcess);

							machineState->available[core] = true;
							machineState->runningProcess[core] = nullptr;
						}
					} else {
						cerr << "Debug, core " << core << ": trying to run a nonexistent process" << endl;
						return 1;
					}
					break;
				case StepAction::IO_REQUEST:
					if (runningProcess != nullptr) {
						if (!runningProcess->ioEvents.empty()) {
							if (runningProcess->ioEvents.front().time == runningProcess->processorTime) {
								IOEvent event = runningProcess->ioEvents.front();
								runningProcess->ioEvents.pop_front();

								runningProcess->state = blocked;
								state->ioModule->submitIORequest(state->time, event, *runningProcess);

								runningProcess = nullptr;
								machineState->runningProcess[core] = nullptr;
								machineState->available[core] = true;
							} else {
								cerr << "Debug, core " << core << ": handling io request at wrong time" << endl;
								return 1;
							}
						} else {
							cerr << "Debug, core " << core << ": handling nonexistent io request" << endl;
							return 1;
						}
					} else {
						cerr << "Debug, core " << core << ": No running process... somehow" << endl;
						return 1;
					}
					break;
				case StepAction::COMPLETE:
					if (runningProcess != nullptr) {
						if (runningProcess->processorTime == runningProcess->reqProcessorTime) {
							runningProcess->state = done;
							runningProcess->doneTime = state->time;

							runningProcess = nullptr;
							machineState->runningProcess[core] = nullptr;
							machineState->available[core] = true;
						} else {
							cerr << "Debug, core " << core << ": running process completing at incorrect time" << endl;
						}
					} else {
						cerr << "Debug, core " << core << ": no running process to complete... somehow" << endl;
						return 1;
					}
					break;
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
		jssleep(machineState->clockDelay);
	}

	return 0;
}

void initOS(uint numCores, SchedulingStrategy strategy) {
	state = new OSState();

	state->stepAction = new StepAction[numCores];
	state->time = 0;
	state->ioModule = new IOModule(state->interrupts);
	state->paused = false;
	state->time = 0;

	state->strategy = strategy;
	if (strategy == SchedulingStrategy::MLF) {
		state->mlfLists = new queue<Process*>[NUM_LEVELS];
	} else {
		state->mlfLists = nullptr;
	}
}

void cleanupOS() {
	for (auto it = state->processList.begin(); it != state->processList.end(); it++) {
		delete *it;
	}

	if (state->strategy == SchedulingStrategy::MLF) {
		delete[] state->mlfLists;
	}

	delete[] state->stepAction;
	delete state->ioModule;
	delete state;
}

Process* schedule(uint core) {
	switch (state->strategy) {
		case SchedulingStrategy::FIFO:
			if (!state->fifoReadyList.empty()) {
				Process* proc = state->fifoReadyList.front();
				state->fifoReadyList.pop();

				return proc;
			}
			break;
		case SchedulingStrategy::SJF:
			if (!state->sjfReadyList.empty()) {
				Process* proc = state->sjfReadyList.top();
				state->sjfReadyList.pop();

				return proc;
			}
			break;
		case SchedulingStrategy::SRT:
			if (!state->srtReadyList.empty()) {
				Process* proc = state->srtReadyList.top();
				state->srtReadyList.pop();

				return proc;
			}
			break;
		case SchedulingStrategy::MLF:
			for (int i = 0; i < NUM_LEVELS; i++) {
				if (!state->mlfLists[i].empty()) {
					Process* proc = state->mlfLists[i].front();
					state->mlfLists[i].pop();

					if (!machineState->available[core]) {
						Process* runningProcess = machineState->runningProcess[core];

						runningProcess->state = ready;
						runningProcess->processorTimeOnLevel = 0;
						state->mlfLists[runningProcess->level].emplace(runningProcess);

						machineState->available[core] = true;
						machineState->runningProcess[core] = nullptr;
					}

					return proc;
				}
			}
			break;
	}

	return nullptr;
}
