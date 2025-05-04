#include "os.h"

#include "machine.h"
#include "process.h"

using namespace std;

OSState* state = nullptr;

void initOS(uint numCores, SchedulingStrategy strategy) {
	state = new OSState();
	nextPID = 0;

	state->stepAction = new StepAction[numCores];
	state->pendingSyscalls = new Syscall[numCores];
	for (uint i = 0; i < numCores; i++) state->pendingSyscalls[i] = Syscall::SYS_NONE;
	state->runningProcess = new PCB*[numCores];
	for (uint i = 0; i < numCores; i++) state->runningProcess[i] = nullptr;
	state->time = 0;
	state->paused = false;

	state->strategy = strategy;
	if (strategy == SchedulingStrategy::MLF) {
		state->mlfLists = new queue<PCB*>[NUM_LEVELS];
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
	delete[] state->pendingSyscalls;
	delete[] state->runningProcess;	 // should not delete contained pointers since they are deleted when all the processes are deleted
	delete state;
}

PCB* schedule(uint core) {
	switch (state->strategy) {
		case SchedulingStrategy::FIFO:
		case SchedulingStrategy::RT_FIFO:
			if (!state->fifoReadyList.empty()) {
				PCB* proc = state->fifoReadyList.front();
				state->fifoReadyList.pop();

				return proc;
			}
			break;
		case SchedulingStrategy::SJF:
			if (!state->sjfReadyList.empty()) {
				PCB* proc = state->sjfReadyList.top();
				state->sjfReadyList.pop();

				return proc;
			}
			break;
		case SchedulingStrategy::SRT:
			if (!state->srtReadyList.empty()) {
				PCB* proc = state->srtReadyList.top();
				state->srtReadyList.pop();

				return proc;
			}
			break;
		case SchedulingStrategy::MLF:
			for (int i = 0; i < NUM_LEVELS; i++) {
				if (!state->mlfLists[i].empty()) {
					PCB* proc = state->mlfLists[i].front();
					state->mlfLists[i].pop();

					if (!machine->cores[core]->free()) {  // If the selected core is currently running a process (the case where a new process arrived and
														  // pre-empts the currently running process of a core)
						PCB* runningProcess = state->runningProcess[core];	// The currently running process

						// Reset the states
						runningProcess->state = ready;
						runningProcess->processorTimeOnLevel = 0;
						runningProcess->regstate = machine->cores[core]->regstate();  // save the CPU registers
						state->mlfLists[runningProcess->level].emplace(runningProcess);

						// Reset the CPU
						state->runningProcess[core] = nullptr;
						machine->cores[core]->load(NOPROC);
					}

					return proc;
				}
			}
			break;
		case SchedulingStrategy::RT_EDF:
			if (!state->edfReadyList.empty()) {
				PCB* proc = state->edfReadyList.top();
				state->edfReadyList.pop();

				return proc;
			}
			break;
		case SchedulingStrategy::RT_LST:
			if (!state->lstReadyList.empty()) {
				PCB* proc = state->lstReadyList.top();
				state->lstReadyList.pop();

				return proc;
			}
			break;
	}

	return nullptr;
}

void handleInterrupt(Interrupt* interrupt) {
	state->interrupts.push_back(interrupt);
}
