#include "browser-api.h"

#include "process.h"

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

Instruction* exported allocInstructionList(uint instructionCount) { return new Instruction[instructionCount]; }

void exported freeInstructionList(Instruction* ptr) { delete[] ptr; }