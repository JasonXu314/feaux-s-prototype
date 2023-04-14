#ifndef UTILS_H
#define UTILS_H

#include <emscripten.h>

#include <list>
#include <queue>

#include "ioModule.h"
#include "process.h"

#define exported EMSCRIPTEN_KEEPALIVE
#define NUM_LEVELS 6
typedef unsigned int uint;

struct Process;
struct IOInterrupt;

struct ProcessCompat {
	ProcessCompat()
		: id(-1),
		  name(nullptr),
		  arrivalTime(-1),
		  doneTime(-1),
		  reqProcessorTime(-1),
		  processorTime(-1),
		  state(State::ready),
		  numIOEvents(-1),
		  ioEvents(nullptr) {}

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

enum SchedulingStrategy { FIFO, SJF, SRT, MLF };

class SJFComparator {
public:
	bool operator()(Process* a, Process* b) { return a->reqProcessorTime > b->reqProcessorTime; }
};

class SRTComparator {
public:
	bool operator()(Process* a, Process* b) { return (a->reqProcessorTime - a->processorTime) > (b->reqProcessorTime - b->processorTime); }
};

struct OSState {
	std::list<Process*> processList;
	std::list<IOInterrupt> interrupts;
	std::queue<Process*> fifoReadyList;
	std::priority_queue<Process*, std::vector<Process*>, SJFComparator> sjfReadyList;
	std::priority_queue<Process*, std::vector<Process*>, SRTComparator> srtReadyList;
	std::queue<Process*>* mlfLists;
	std::list<Process*> reentryList;
	StepAction* stepAction;
	uint time;
	bool paused;
	SchedulingStrategy strategy;
	IOModule* ioModule;
};

struct OSStateCompat {
	uint numProcesses;
	ProcessCompat* processList;
	uint numInterrupts;
	IOInterrupt* interrupts;
	uint numReady;
	ProcessCompat* readyList;
	uint numReentering;
	ProcessCompat* reentryList;
	StepAction* stepAction;
	uint time;
	bool paused;
};

extern MachineState* machineState;

void exportProcess(const Process& src, ProcessCompat& dest);

extern "C" {
char* exported allocString(unsigned int size);
void exported freeString(char* str);

MachineStateCompat* exported getMachineState();
}

#endif