#include "browser-api.h"

#include "process.h"

MachineStateCompat* exportMachineState = nullptr;
OSStateCompat* exportState = nullptr;

Instruction* exported allocInstructionList(uint instructionCount) { return new Instruction[instructionCount]; }

void exported freeInstructionList(Instruction* ptr) { delete[] ptr; }

char* exported allocString(unsigned int size) {
	char* str = new char[size + 1];

	str[size] = '\0';

	return str;
}

void exported freeString(char* str) { delete[] str; }

uint exported addProcess(Instruction* instructionList, uint size, char* processName) {
	Process* proc = new Process();
	uint ioID = 0;

	proc->id = ++nextPID;
	proc->name = processName;
	proc->arrivalTime = state->time;
	proc->level = 0;
	proc->processorTimeOnLevel = 0;

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
				return -1;
		}
	}

	state->processList.push_back(proc);

	switch (state->strategy) {
		case SchedulingStrategy::FIFO:
			state->fifoReadyList.emplace(proc);
			break;
		case SchedulingStrategy::SJF:
			state->sjfReadyList.emplace(proc);
			break;
		case SchedulingStrategy::SRT:
			state->srtReadyList.emplace(proc);
			break;
		case SchedulingStrategy::MLF:
			state->mlfLists[0].emplace(proc);
			break;
		default:
			return -1;
	}

	return proc->id;
}

void exported pause() { state->paused = true; }
void exported unpause() { state->paused = false; }
void exported setClockDelay(uint delay) { machineState->clockDelay = delay; }

void exported setSchedulingStrategy(SchedulingStrategy strategy) {
	cleanupOS();

	for (uint i = 0; i < machineState->numCores; i++) {
		machineState->available[i] = true;
		machineState->runningProcess[i] = nullptr;
	}

	initOS(machineState->numCores, strategy);
}

MachineStateCompat* exported getMachineState() {
	static uint prevNumCores = 0;

	if (exportMachineState == nullptr) {
		exportMachineState = new MachineStateCompat();
	}

	if (exportMachineState->runningProcess != nullptr) {
		for (uint i = 0; i < prevNumCores; i++) {
			delete[] exportMachineState->runningProcess[i].ioEvents;
		}

		delete[] exportMachineState->runningProcess;
	}

	exportMachineState->numCores = machineState->numCores;
	exportMachineState->clockDelay = machineState->clockDelay;
	exportMachineState->available = machineState->available;

	exportMachineState->runningProcess = new ProcessCompat[machineState->numCores];
	for (uint i = 0; i < machineState->numCores; i++) {
		exportProcess(*machineState->runningProcess[i], exportMachineState->runningProcess[i]);
	}

	prevNumCores = machineState->numCores;

	return exportMachineState;
}

OSStateCompat* exported getOSState() {
	static uint prevProcListSize = 0, prevReadyListSize = 0, prevReentryListSize = 0, prevMLFReadyListSizes[NUM_LEVELS] = {0};

	if (exportState == nullptr) {
		exportState = new OSStateCompat();
		// cout << (uintptr_t)&exportState->mlfReadyLists - (uintptr_t)&exportState->mlfNumReady << endl;
	}

	if (exportState->processList != nullptr) {
		for (uint i = 0; i < prevProcListSize; i++) {
			delete[] exportState->processList[i].ioEvents;
		}

		delete[] exportState->processList;
		exportState->processList = nullptr;
	}
	if (exportState->interrupts != nullptr) {
		delete[] exportState->interrupts;
		exportState->interrupts = nullptr;
	}
	if (exportState->readyList != nullptr) {
		for (uint i = 0; i < prevReadyListSize; i++) {
			delete[] exportState->readyList[i].ioEvents;
		}

		delete[] exportState->readyList;
		exportState->readyList = nullptr;
	}
	if (exportState->reentryList != nullptr) {
		for (uint i = 0; i < prevReentryListSize; i++) {
			delete[] exportState->reentryList[i].ioEvents;
		}

		delete[] exportState->reentryList;
		exportState->reentryList = nullptr;
	}
	if (exportState->stepAction != nullptr) {
		delete[] exportState->stepAction;
		exportState->stepAction = nullptr;
	}
	for (uint i = 0; i < NUM_LEVELS; i++) {
		if (exportState->mlfReadyLists[i] != nullptr) {
			for (uint j = 0; j < prevMLFReadyListSizes[i]; j++) {
				delete[] exportState->mlfReadyLists[i][j].ioEvents;
			}

			delete[] exportState->mlfReadyLists[i];
			exportState->mlfReadyLists[i] = nullptr;
		}
	}

	uint i = 0;
	exportState->numProcesses = state->processList.size();
	if (exportState->numProcesses > 0) {
		exportState->processList = new ProcessCompat[exportState->numProcesses];
		for (auto it = state->processList.begin(); it != state->processList.end(); it++, i++) {
			exportProcess(**it, exportState->processList[i]);
		}

		prevProcListSize = exportState->numProcesses;
	} else {
		prevProcListSize = 0;
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
		exportState->interrupts = nullptr;	// should be ignored on the other end if there are 0 interrupts, but set it to nullptr anyway for insurance
	}

	switch (state->strategy) {
		case SchedulingStrategy::FIFO:
			exportState->numReady = state->fifoReadyList.size();
			if (exportState->numReady > 0) {
				exportState->readyList = new ProcessCompat[exportState->numReady];
				for (uint i = 0; i < exportState->numReady; i++) {
					Process* ptr = state->fifoReadyList.front();

					exportProcess(*ptr, exportState->readyList[i]);

					state->fifoReadyList.pop();
					state->fifoReadyList.push(ptr);
				}

				prevReadyListSize = exportState->numReady;
			} else {
				prevReadyListSize = 0;
				exportState->readyList = nullptr;  // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
			}
			break;
		case SchedulingStrategy::SJF:
			exportState->numReady = state->sjfReadyList.size();
			if (exportState->numReady > 0) {
				exportState->readyList = new ProcessCompat[exportState->numReady];
				list<Process*> copy;
				for (uint i = 0; i < exportState->numReady; i++) {
					Process* ptr = state->sjfReadyList.top();

					exportProcess(*ptr, exportState->readyList[i]);

					state->sjfReadyList.pop();
					copy.push_back(ptr);
				}
				for (auto it = copy.begin(); it != copy.end(); it++) {
					state->sjfReadyList.push(*it);
				}

				prevReadyListSize = exportState->numReady;
			} else {
				prevReadyListSize = 0;
				exportState->readyList = nullptr;  // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
			}
			break;
		case SchedulingStrategy::SRT:
			exportState->numReady = state->srtReadyList.size();
			if (exportState->numReady > 0) {
				exportState->readyList = new ProcessCompat[exportState->numReady];
				list<Process*> copy;
				for (uint i = 0; i < exportState->numReady; i++) {
					Process* ptr = state->srtReadyList.top();

					exportProcess(*ptr, exportState->readyList[i]);

					state->srtReadyList.pop();
					copy.push_back(ptr);
				}
				for (auto it = copy.begin(); it != copy.end(); it++) {
					state->srtReadyList.push(*it);
				}

				prevReadyListSize = exportState->numReady;
			} else {
				prevReadyListSize = 0;
				exportState->readyList = nullptr;  // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
			}
			break;
		case SchedulingStrategy::MLF:
			for (uint i = 0; i < NUM_LEVELS; i++) {
				uint size = state->mlfLists[i].size();

				exportState->mlfNumReady[i] = size;

				if (size > 0) {
					exportState->mlfReadyLists[i] = new ProcessCompat[size];
					for (uint j = 0; j < size; j++) {
						Process* ptr = state->mlfLists[i].front();

						exportProcess(*ptr, exportState->mlfReadyLists[i][j]);

						state->mlfLists[i].pop();
						state->mlfLists[i].push(ptr);
					}

					prevMLFReadyListSizes[i] = size;
				} else {
					prevMLFReadyListSizes[i] = 0;
					exportState->mlfReadyLists[i] =
						nullptr;  // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
				}
			}

			break;
	}

	exportState->numReentering = state->reentryList.size();
	if (exportState->numReentering > 0) {
		i = 0;
		exportState->reentryList = new ProcessCompat[exportState->numReentering];
		for (auto it = state->reentryList.begin(); it != state->reentryList.end(); it++, i++) {
			exportProcess(**it, exportState->reentryList[i]);
		}

		prevReentryListSize = exportState->numReentering;
	} else {
		prevReentryListSize = 0;
		exportState->reentryList = nullptr;	 // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
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