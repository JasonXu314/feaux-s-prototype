#include "browser-api.h"

#include "machine.h"

MachineStateCompat* exportMachineState = nullptr;
OSStateCompat* exportState = nullptr;

Instruction*
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	allocInstructionList(uint instructionCount) {
	return new Instruction[instructionCount];
}

void
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	freeInstructionList(Instruction* ptr) {
	delete[] ptr;
}

char*
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	allocString(unsigned int size) {
	char* str = new char[size + 1];

	str[size] = '\0';

	return str;
}

void
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	freeString(char* str) {
	delete[] str;
}

void
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	loadProgram(Instruction* instructionList, uint size, char* name) {
	Program newProgram{name, size, new Instruction[size]};

	for (uint i = 0; i < size; i++) {
		newProgram.instructions[i] = instructionList[i];
	}

	state->programs.emplace(name, newProgram);
}

Instruction*
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	getProgramLocation(char* name) {
	return state->programs.at(name).instructions;
}

uint
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	spawn(const char* name, uint d) {
	if (state->programs.count(name)) {	// If there exists a program of that name
		Program& program = state->programs.at(name);
		PCB* proc = new PCB();

		proc->pid = ++nextPID;
		proc->name = name;
		proc->arrivalTime = state->time;
		proc->deadline = d == -1 ? -1 : state->time + d;
		proc->level = 0;
		proc->processorTimeOnLevel = 0;
		proc->state = ready;

		memset(&proc->regstate, 0, sizeof(Registers));
#if FEAUX_S_BENCHMARKING
		proc->regstate.rip = (uint64_t)program.instructions;  // Loads the address of the first instruction into the instruction pointer of the process
#else
		proc->regstate.rip = (uint)program.instructions;  // Loads the address of the first instruction into the instruction pointer of the process
#endif
		proc->regstate.rdi = 0;
		proc->reqProcessorTime = program.length - 1;

		state->processList.push_back(proc);

		switch (state->strategy) {
			case SchedulingStrategy::FIFO:
			case SchedulingStrategy::RT_FIFO:
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
			case SchedulingStrategy::RT_EDF:
				state->edfReadyList.emplace(proc);
				break;
			case SchedulingStrategy::RT_LST:
				state->lstReadyList.emplace(proc);
				break;
			default:
				return -1;
		}

		return proc->pid;
	} else {
		return -1;
	}
}

void
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	dispatch(const char* name, uint p, uint d, uint s) {
	if (state->programs.count(name)) {	// If there exists a program of that name
		RTJob* job = new RTJob();

		job->program = name;
		job->period = p;
		job->deadline = d;
		job->delay = state->time + s;

		state->jobList.emplace_back(job);
	}
}

void
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	pause() {
	state->paused = true;
}
void
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	unpause() {
	state->paused = false;
}
void
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	setClockDelay(uint delay) {
	machine->clockDelay = delay;
}

void
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	setNumCores(uint8_t cores) {
	map<string, Program> programs = state->programs;
	SchedulingStrategy strategy = state->strategy;
	uint8_t numIODevices = machine->numIODevices;
	uint clockDelay = machine->clockDelay;
	cleanupOS();
	cleanupMachine();

	initMachine(cores, numIODevices);
	machine->clockDelay = clockDelay;
	initOS(machine->numCores, strategy);
	state->programs = programs;
}

void
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	setNumIODevices(uint8_t ioDevices) {
	map<string, Program> programs = state->programs;
	SchedulingStrategy strategy = state->strategy;
	uint8_t numCores = machine->numCores;
	uint clockDelay = machine->clockDelay;
	cleanupOS();
	cleanupMachine();

	initMachine(numCores, ioDevices);
	machine->clockDelay = clockDelay;
	initOS(machine->numCores, strategy);
	state->programs = programs;
}

void
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	setSchedulingStrategy(SchedulingStrategy strategy) {
	map<string, Program> programs = state->programs;  // Save a copy of the programs, so that the new OS will still have the same programs
	cleanupOS();

	for (uint i = 0; i < machine->numCores; i++) {
		machine->cores[i]->load(NOPROC);
	}

	for (uint i = 0; i < machine->numIODevices; i++) {
		machine->ioDevices[i]->clear();
	}

	initOS(machine->numCores, strategy);
	state->programs = programs;
}

MachineStateCompat*
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	getMachineState() {
	static uint prevNumCores = 0, prevNumIODevices = 0;

	if (exportMachineState == nullptr) {
		exportMachineState = new MachineStateCompat();	// Init exported data
														// cout << (uintptr_t)&exportMachineState->clockDelay - (uintptr_t)exportMachineState << endl;
	}

	// cout << "Exporting primitives" << endl;
	exportMachineState->numCores = machine->numCores;
	exportMachineState->numIODevices = machine->numIODevices;
	exportMachineState->clockDelay = machine->clockDelay;

	// cout << "Clearing memory" << endl;
	if (machine->numCores != prevNumCores) {
		delete[] exportMachineState->cores;
		exportMachineState->cores = new CPUState[machine->numCores];
	}

	if (machine->numIODevices != prevNumIODevices) {
		delete[] exportMachineState->ioDevices;
		exportMachineState->ioDevices = new DeviceState[machine->numIODevices];
	}

	// cout << "Exporting CPUs" << endl;
	for (uint i = 0; i < machine->numCores; i++) {
		exportCPU(*machine->cores[i], exportMachineState->cores[i]);
	}

	// cout << "Exporting IO Devices" << endl;
	for (uint i = 0; i < machine->numIODevices; i++) {
		exportIODevice(*machine->ioDevices[i], exportMachineState->ioDevices[i]);
	}

	prevNumCores = machine->numCores;
	prevNumIODevices = machine->numIODevices;

	return exportMachineState;
}

OSStateCompat*
#ifndef FEAUX_S_BENCHMARKING
	exported
#endif
	getOSState() {
	static uint prevProcListSize = 0, prevReadyListSize = 0, prevReentryListSize = 0, prevMLFReadyListSizes[NUM_LEVELS] = {0};

	if (exportState == nullptr) {
		exportState = new OSStateCompat();
		// cout << (uintptr_t)&exportState->mlfReadyLists - (uintptr_t)&exportState->mlfNumReady << endl;
	}

	// cout << "Clearing memory" << endl;
	if (exportState->processList != nullptr) {
		delete[] exportState->processList;
		exportState->processList = nullptr;
	}
	if (exportState->interrupts != nullptr) {
		delete[] exportState->interrupts;
		exportState->interrupts = nullptr;
	}
	if (exportState->readyList != nullptr) {
		delete[] exportState->readyList;
		exportState->readyList = nullptr;
	}
	if (exportState->reentryList != nullptr) {
		delete[] exportState->reentryList;
		exportState->reentryList = nullptr;
	}
	if (exportState->stepAction != nullptr) {
		delete[] exportState->stepAction;
		exportState->stepAction = nullptr;
	}
	for (uint i = 0; i < NUM_LEVELS; i++) {
		if (exportState->mlfReadyLists[i] != nullptr) {
			delete[] exportState->mlfReadyLists[i];
			exportState->mlfReadyLists[i] = nullptr;
		}
	}
	if (exportState->pendingRequests != nullptr) {
		delete[] exportState->pendingRequests;
		exportState->pendingRequests = nullptr;
	}
	if (exportState->pendingSyscalls != nullptr) {
		delete[] exportState->pendingSyscalls;
		exportState->pendingSyscalls = nullptr;
	}
	if (exportState->runningProcesses != nullptr) {
		delete[] exportState->runningProcesses;
		exportState->runningProcesses = nullptr;
	}
	// cout << "Cleared memory, exporting process list" << endl;

	uint i = 0;
	exportState->numProcesses = state->processList.size();
	if (exportState->numProcesses > 0) {  // If there exist processes, export them
		exportState->processList = new ProcessCompat[exportState->numProcesses];
		for (auto it = state->processList.begin(); it != state->processList.end(); it++, i++) {
			exportProcess(**it, exportState->processList[i]);
		}

		prevProcListSize = exportState->numProcesses;
	} else {
		prevProcListSize = 0;
		exportState->processList = nullptr;	 // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
	}

	// cout << "Exporting interrupts" << endl;
	exportState->numInterrupts = state->interrupts.size();
	if (exportState->numInterrupts > 0) {  // If there exist interrupts, export them
		i = 0;
		exportState->interrupts = new InterruptCompat[exportState->numInterrupts];
		for (auto it = state->interrupts.begin(); it != state->interrupts.end(); it++, i++) {
			exportInterrupt(**it, exportState->interrupts[i]);
		}
	} else {
		exportState->interrupts = nullptr;	// should be ignored on the other end if there are 0 interrupts, but set it to nullptr anyway for insurance
	}

	// cout << "Exporting ready list(s)" << endl;
	switch (state->strategy) {
		case SchedulingStrategy::FIFO:
		case SchedulingStrategy::RT_FIFO:
			exportState->numReady = state->fifoReadyList.size();
			if (exportState->numReady > 0) {
				exportState->readyList = new ProcessCompat[exportState->numReady];
				for (uint i = 0; i < exportState->numReady; i++) {
					PCB* ptr = state->fifoReadyList.front();

					exportProcess(*ptr, exportState->readyList[i]);

					// Pop and requeue to access the next process in the list
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
				// Need to create copy of list in order to access all elements (priority queue does not allow iteration)
				list<PCB*> copy;
				for (uint i = 0; i < exportState->numReady; i++) {
					PCB* ptr = state->sjfReadyList.top();

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
				// Need to create copy of list in order to access all elements (priority queue does not allow iteration)
				list<PCB*> copy;
				for (uint i = 0; i < exportState->numReady; i++) {
					PCB* ptr = state->srtReadyList.top();

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
						PCB* ptr = state->mlfLists[i].front();

						exportProcess(*ptr, exportState->mlfReadyLists[i][j]);

						// Pop and requeue to access the next process in the list
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
		case SchedulingStrategy::RT_EDF:
			exportState->numReady = state->edfReadyList.size();
			if (exportState->numReady > 0) {
				exportState->readyList = new ProcessCompat[exportState->numReady];

				auto copy = state->edfReadyList;
				for (uint i = 0; i < exportState->numReady; i++) {
					PCB* ptr = copy.top();

					exportProcess(*ptr, exportState->readyList[i]);

					copy.pop();
				}

				prevReadyListSize = exportState->numReady;
			} else {
				prevReadyListSize = 0;
				exportState->readyList = nullptr;  // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
			}
			break;
		case SchedulingStrategy::RT_LST:
			exportState->numReady = state->lstReadyList.size();
			if (exportState->numReady > 0) {
				exportState->readyList = new ProcessCompat[exportState->numReady];

				auto copy = state->lstReadyList;
				for (uint i = 0; i < exportState->numReady; i++) {
					PCB* ptr = copy.top();

					exportProcess(*ptr, exportState->readyList[i]);

					copy.pop();
				}

				prevReadyListSize = exportState->numReady;
			} else {
				prevReadyListSize = 0;
				exportState->readyList = nullptr;  // should be ignored on the other end if there are 0 processes, but set it to nullptr anyway for insurance
			}
			break;
	}

	// cout << "Exporting reentry list" << endl;
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

	// cout << "Exporting actions" << endl;
	i = 0;
	exportState->stepAction = new StepAction[machine->numCores];
	for (; i < machine->numCores; i++) {
		exportState->stepAction[i] = state->stepAction[i];
	}

	// cout << "Exporting IO requests" << endl;
	exportState->numRequests = state->pendingRequests.size();
	if (exportState->numRequests > 0) {
		i = 0;
		exportState->pendingRequests = new IORequest[exportState->numRequests];
		for (; i < exportState->numRequests; i++) {
			exportState->pendingRequests[i] = state->pendingRequests.front();

			// Pop and requeue to access the next process in the list
			state->pendingRequests.push(state->pendingRequests.front());
			state->pendingRequests.pop();
		}
	}

	// cout << "Exporting syscalls" << endl;
	i = 0;
	exportState->pendingSyscalls = new Syscall[machine->numCores];
	for (; i < machine->numCores; i++) {
		exportState->pendingSyscalls[i] = state->pendingSyscalls[i];
	}

	// cout << "Exporting running processes" << endl;
	i = 0;
	exportState->runningProcesses = new ProcessCompat[machine->numCores];
	for (; i < machine->numCores; i++) {
		if (state->runningProcess[i] != nullptr) {
			exportProcess(*state->runningProcess[i], exportState->runningProcesses[i]);
		} else {
			// Export nonexistent process
			exportState->runningProcesses[i].pid = -1;
		}
	}

	exportState->time = state->time;
	exportState->paused = state->paused;

	return exportState;
}