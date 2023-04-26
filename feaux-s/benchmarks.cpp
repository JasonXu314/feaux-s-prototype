#include "benchmarks.h"

using namespace std;

struct {
	double usedCPUTime;
	double totalCPUTime;
} stats;

void printStats() {
	cout << "Strategy: " << STRATEGY_NAME(state->strategy) << endl;

	double totalTT = 0, maxTT = -INFINITY, minTT = INFINITY;
	for (auto it = state->processList.begin(); it != state->processList.end(); it++) {
		double tt = (*it)->doneTime - (*it)->arrivalTime;
		totalTT += tt;

		if (tt < minTT) {
			minTT = tt;
		}
		if (tt > maxTT) {
			maxTT = tt;
		}
	}
	double att = totalTT / state->processList.size();

	cout << "ATT: " << att << " quanta\n"
		 << "CPU Utilization: " << stats.usedCPUTime / stats.totalCPUTime * 100 << "%\n"
		 << "Max TT: " << maxTT << " quanta\n"
		 << "Min TT: " << minTT << " quanta\n"
		 << endl;
}

#if FEAUX_S_BENCHMARKING == 1
bool simulate() {
	stats.totalCPUTime += 2;
	stats.usedCPUTime += !machine->cores[0]->free() + !machine->cores[1]->free();

	if (state->time == 1) {
		Instruction workerInstructions[10] = {
			{Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0},
			{Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::EXIT, 0, 0},
		};

		char workerName[] = "worker";
		loadProgram(workerInstructions, 10, workerName);

		stats.usedCPUTime = 0;
		stats.totalCPUTime = 0;

		spawn(workerName);
		spawn(workerName);
		spawn(workerName);
		spawn(workerName);
		spawn(workerName);
		return true;
	}
	return false;
}
#elif FEAUX_S_BENCHMARKING == 2
bool simulate() {
	stats.totalCPUTime += 2;
	stats.usedCPUTime += !machine->cores[0]->free() + !machine->cores[1]->free();

	if (state->time == 1) {
		Instruction shortWorkerInstructions[10] =
			{
				{Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0},
				{Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::EXIT, 0, 0},
			},
					longWorkerInstructions[256];

		for (int i = 0; i < 255; i++) {
			longWorkerInstructions[i].opcode = Opcode::WORK;
			longWorkerInstructions[i].operand1 = 0;
			longWorkerInstructions[i].operand2 = 0;
		}
		longWorkerInstructions[255].opcode = Opcode::EXIT;
		longWorkerInstructions[255].operand1 = 0;
		longWorkerInstructions[255].operand2 = 0;

		char workerName[] = "worker", longWorkerName[] = "long worker";
		loadProgram(shortWorkerInstructions, 10, workerName);
		loadProgram(longWorkerInstructions, 256, longWorkerName);

		stats.usedCPUTime = 0;
		stats.totalCPUTime = 0;

		spawn(longWorkerName);
		spawn(longWorkerName);
		spawn(workerName);
		return true;
	}

	if (state->time % 10 == 0 && state->time <= 300) {
		char workerName[] = "worker";
		spawn(workerName);
	}

	if (state->time <= 300) {
		return true;
	}

	return false;
}
#elif FEAUX_S_BENCHMARKING == 3
bool simulate() {
	stats.totalCPUTime += 2;
	stats.usedCPUTime += !machine->cores[0]->free() + !machine->cores[1]->free();

	if (state->time == 1) {
		Instruction workerInstructions[5] =
			{
				{Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::IO, 5, 0}, {Opcode::EXIT, 0, 0},
			},
					shortWorkerInstructions[3] = {{Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::EXIT, 0, 0}},
					longWorkerInstructions[10] = {
						{Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0},
						{Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::WORK, 0, 0}, {Opcode::EXIT, 0, 0},
					};

		char workerName[] = "worker", shortWorkerName[] = "short worker", longWorkerName[] = "long worker";
		loadProgram(workerInstructions, 10, workerName);
		loadProgram(shortWorkerInstructions, 10, shortWorkerName);
		loadProgram(longWorkerInstructions, 10, longWorkerName);

		stats.usedCPUTime = 0;
		stats.totalCPUTime = 0;

		spawn(workerName);
		spawn(workerName);
		spawn(longWorkerName);
		spawn(longWorkerName);
		return true;
	}

	if (state->time % 2 == 0 && state->time <= 6) {
		char name[] = "short worker";
		spawn(name);
		spawn(name);
	}

	if (state->time <= 6) {
		return true;
	}

	return false;
}
#endif