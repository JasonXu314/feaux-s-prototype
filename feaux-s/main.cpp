#include <emscripten.h>

#include <list>
#include <queue>

#include "ioModule.h"
#include "process.h"
#include "utils.h"

// clang-format off
EM_JS(void, jssleep, (int milis), { Asyncify.handleSleep(wakeUp => { setTimeout(wakeUp, milis); }); });
// clang-format on

extern "C" {
void exported addProcess(Instruction* instructionList, uint size, char* processName);
void exported pause();
void exported unpause();

OSStateCompat* exported getOSState();
}

uint nextPID = 0;

OSState* state = nullptr;
OSStateCompat* exportState = nullptr;

void initOS();

int main(int argc, char* argv[]) {
	cout << sizeof(ProcessCompat) << endl;

	machineState = new MachineState{4, 250, new bool[4]{true, true, true, true}, new Process*[4]};

	initOS();

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
				} else if (!state->readyList.empty()) {
					state->stepAction[core] = StepAction::BEGIN_RUN;  // start running a process
				}
			} else {
				if (runningProcess->processorTime == runningProcess->reqProcessorTime) {
					state->stepAction[core] = StepAction::COMPLETE;	 // running process is finished
				} else if (!runningProcess->ioEvents.empty() && runningProcess->ioEvents.front().time == runningProcess->processorTime) {
					state->stepAction[core] = StepAction::IO_REQUEST;  // running process issued an io request
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
							state->readyList.emplace(originProcess);
						}
					} else {
						cerr << "Debug, core " << core << ": trying to handle nonexistent interrupt" << endl;
						return 1;
					}
					break;
				case StepAction::BEGIN_RUN:
					if (!state->readyList.empty()) {
						machineState->runningProcess[core] = state->readyList.front();
						state->readyList.pop();

						runningProcess->state = processing;
						runningProcess->processorTime++;
						machineState->available[core] = false;
					} else {
						cerr << "Debug, core " << core << ": Attempting to run a nonexistent process" << endl;
						return 1;
					}
					break;
				case StepAction::CONTINUE_RUN:
					if (runningProcess != nullptr) {
						runningProcess->processorTime++;
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

	skip:
		jssleep(machineState->clockDelay);
	}

	return 0;
}

void initOS() {
	state = new OSState();

	state->stepAction = new StepAction[4];
	state->time = 0;
	state->ioModule = new IOModule(state->interrupts);
}

void exported addProcess(Instruction* instructionList, uint size, char* processName) {
	Process* proc = new Process();
	uint ioID = 0;

	proc->id = ++nextPID;
	proc->name = processName;
	proc->arrivalTime = state->time;

	for (uint i = 0; i < size; i++) {
		Instruction& instruction = instructionList[i];

		switch (instruction.opcode) {
			case Opcode::NOP:
				proc->reqProcessorTime++;
				break;
			case Opcode::WORK:
				proc->reqProcessorTime += instruction.operand;
				break;
			case Opcode::IO:
				proc->ioEvents.push_back(IOEvent(proc->reqProcessorTime, instruction.operand, ioID++));
				break;
			default:
				return;
		}
	}

	state->processList.push_back(proc);
	state->readyList.emplace(proc);
}

void exported pause() { state->paused = true; }
void exported unpause() { state->paused = false; }

OSStateCompat* exported getOSState() {
	static uint prevProcListSize = 0;

	if (exportState == nullptr) {
		exportState = new OSStateCompat();
	}

	if (exportState->processList != nullptr) {
		for (uint i = 0; i < prevProcListSize; i++) {
			delete[] exportState->processList[i].ioEvents;
		}

		delete[] exportState->processList;
	}
	if (exportState->interrupts != nullptr) delete[] exportState->interrupts;
	if (exportState->readyList != nullptr) delete[] exportState->readyList;
	if (exportState->stepAction != nullptr) delete[] exportState->stepAction;

	uint i = 0;
	exportState->numProcesses = state->processList.size();
	if (exportState->numProcesses > 0) {
		exportState->processList = new ProcessCompat[exportState->numProcesses];
		for (auto it = state->processList.begin(); it != state->processList.end(); it++, i++) {
			Process& proc = **it;
			ProcessCompat& copy = exportState->processList[i];

			copy.id = proc.id;
			copy.name = proc.name.c_str();
			copy.arrivalTime = proc.arrivalTime;
			copy.doneTime = proc.doneTime;
			copy.reqProcessorTime = proc.reqProcessorTime;
			copy.processorTime = proc.processorTime;
			copy.state = proc.state;

			copy.numIOEvents = proc.ioEvents.size();

			if (copy.numIOEvents > 0) {
				copy.ioEvents = new IOEvent[copy.numIOEvents];

				uint j = 0;
				for (auto it = proc.ioEvents.begin(); it != proc.ioEvents.end(); it++) {
					copy.ioEvents[j++] = *it;
				}
			} else {
				copy.ioEvents = nullptr;  // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
			}
		}
		prevProcListSize = state->processList.size();
	} else {
		prevProcListSize = exportState->numProcesses;
		exportState->processList = nullptr;	 // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
	}

	exportState->numInterrupts = state->interrupts.size();
	if (exportState->numInterrupts > 0) {
		i = 0;
		exportState->interrupts = new IOInterrupt[exportState->numInterrupts];
		for (auto it = state->interrupts.begin(); it != state->interrupts.end(); it++, i++) {
			exportState->interrupts[i] = *it;
		}
	} else {
		exportState->interrupts = nullptr;	// should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
	}

	exportState->numReady = state->readyList.size();
	if (exportState->numReady > 0) {
		i = 0;
		exportState->readyList = new ProcessCompat[exportState->numReady];
		for (; i < exportState->numReady; i++) {
			Process* ptr = state->readyList.front();
			Process& proc = *ptr;
			ProcessCompat& copy = exportState->processList[i];

			copy.id = proc.id;
			copy.name = proc.name.c_str();
			copy.arrivalTime = proc.arrivalTime;
			copy.doneTime = proc.doneTime;
			copy.reqProcessorTime = proc.reqProcessorTime;
			copy.processorTime = proc.processorTime;
			copy.state = proc.state;

			copy.numIOEvents = proc.ioEvents.size();
			if (copy.numIOEvents > 0) {
				copy.ioEvents = new IOEvent[copy.numIOEvents];

				uint j = 0;
				for (auto ioEvtIt = proc.ioEvents.begin(); ioEvtIt != proc.ioEvents.end(); ioEvtIt++) {
					copy.ioEvents[j++] = *ioEvtIt;
				}
			} else {
				copy.ioEvents = nullptr;  // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
			}

			state->readyList.pop();
			state->readyList.emplace(ptr);
		}
	} else {
		exportState->readyList = nullptr;  // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
	}

	i = 0;
	exportState->stepAction = new StepAction[machineState->numCores];
	for (; i < machineState->numCores; i++) {
		exportState->stepAction[i] = state->stepAction[i];
	}

	exportState->time = state->time;
	exportState->paused = state->paused;

	return exportState;
}