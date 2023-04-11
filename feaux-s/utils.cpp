#include "utils.h"

MachineState* machineState = nullptr;
MachineStateCompat* exportMachineState = nullptr;

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
		Process& proc = *machineState->runningProcess[i];
		ProcessCompat& copy = exportMachineState->runningProcess[i];

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

	prevNumCores = machineState->numCores;

	return exportMachineState;
}