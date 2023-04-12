#include "utils.h"

MachineState* machineState = nullptr;
MachineStateCompat* exportMachineState = nullptr;

void exportProcess(const Process& src, ProcessCompat& dest) {
	dest.id = src.id;
	dest.name = src.name.c_str();
	dest.arrivalTime = src.arrivalTime;
	dest.doneTime = src.doneTime;
	dest.reqProcessorTime = src.reqProcessorTime;
	dest.processorTime = src.processorTime;
	dest.state = src.state;

	dest.numIOEvents = src.ioEvents.size();

	if (dest.numIOEvents > 0) {
		dest.ioEvents = new IOEvent[dest.numIOEvents];

		uint j = 0;
		for (auto it = src.ioEvents.begin(); it != src.ioEvents.end(); it++) {
			dest.ioEvents[j++] = *it;
		}
	} else {
		dest.ioEvents = nullptr;  // should be ignored on the other end if there are 0 IO Events, but set it to nullptr anyway for insurance
	}
}

char* exported allocString(unsigned int size) {
	char* str = new char[size + 1];

	str[size] = '\0';

	return str;
}

void exported freeString(char* str) { delete[] str; }

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