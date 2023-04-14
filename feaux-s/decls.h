#ifndef DECLS_H
#define DECLS_H

#include <emscripten.h>

#include <list>
#include <queue>

struct Process;
struct IOEvent;
struct IOInterrupt;
class IOModule;

#define exported EMSCRIPTEN_KEEPALIVE
#define NUM_LEVELS 6
typedef unsigned int uint;

enum StepAction { NOOP, HANDLE_INTERRUPT, BEGIN_RUN, CONTINUE_RUN, IO_REQUEST, COMPLETE };
enum SchedulingStrategy { FIFO, SJF, SRT, MLF };
enum State { ready, processing, blocked, done };  // Used to track the process states
enum Opcode { NOP, WORK, IO };

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
class SJFComparator {
public:
	bool operator()(Process* a, Process* b);
};

class SRTComparator {
public:
	bool operator()(Process* a, Process* b);
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
	uint* mlfNumReady;
	ProcessCompat* mlfReadyLists[NUM_LEVELS] = {nullptr};
	StepAction* stepAction;
	uint time;
	bool paused;
};

struct Instruction {
	uint8_t opcode;
	uint8_t operand;
};

extern MachineState* machineState;
extern OSState* state;
extern uint nextPID;

#endif