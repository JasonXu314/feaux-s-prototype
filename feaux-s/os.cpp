#include "os.h"

#include "machine.h"
#include "process.h"

using namespace std;

OSState* state = nullptr;

void initOS(uint numCores, SchedulingStrategy strategy) {
	state = new OSState();

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

					if (!machine->cores[core]->free()) {
						PCB* runningProcess = state->runningProcess[core];

						runningProcess->state = ready;
						runningProcess->processorTimeOnLevel = 0;
						state->mlfLists[runningProcess->level].emplace(runningProcess);

						state->runningProcess[core] = nullptr;
						runningProcess->regstate = machine->cores[core]->regstate();
						Registers noop{0};
						machine->cores[core]->load(noop);
					}

					return proc;
				}
			}
			break;
	}

	return nullptr;
}

void handleInterrupt(Interrupt* interrupt) { state->interrupts.push_back(interrupt); }
