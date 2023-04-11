#ifndef UTILS_H
#define UTILS_H

#include <emscripten.h>

#include <list>
#include <queue>

#include "ioModule.h"
#include "process.h"

#define exported EMSCRIPTEN_KEEPALIVE
typedef unsigned int uint;

struct Process;
struct IOInterrupt;

struct ProcessCompat {
	uint id;
	const char* name;
	long arrivalTime;
	long doneTime;
	long reqProcessorTime;
	long processorTime;
	State state;
	uint numIOEvents;
	IOEvent* ioEvents;
};

struct MachineState {
	uint8_t numCores;
	uint clockDelay;
	bool* available;
	Process** runningProcess;
};

struct MachineStateCompat {
	uint8_t numCores;
	uint clockDelay;
	bool* available;
	ProcessCompat* runningProcess;
};

enum StepAction { NOOP, HANDLE_INTERRUPT, BEGIN_RUN, CONTINUE_RUN, IO_REQUEST, COMPLETE };

struct OSState {
	std::list<Process*> processList;
	std::list<IOInterrupt> interrupts;
	std::queue<Process*> readyList;
	StepAction* stepAction;
	uint time;
	bool paused;
	IOModule* ioModule;
};

struct OSStateCompat {
	ProcessCompat* processList;
	IOInterrupt* interrupts;
	ProcessCompat* readyList;
	StepAction* stepAction;
	uint time;
	bool paused;
};

extern MachineState* machineState;

extern "C" {
char* exported allocString(unsigned int size);
void exported freeString(char* str);

MachineStateCompat* exported getMachineState();
}

#endif