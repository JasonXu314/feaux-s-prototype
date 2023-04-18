#ifndef BROWSER_API_H
#define BROWSER_API_H

#include "decls.h"
#include "utils.h"

extern void initOS(uint numCores, SchedulingStrategy strategy);
extern void cleanupOS();

struct CPUState {
	bool available;
	Registers regstate;
};

struct DeviceState {
	uint pid;
	uint duration;
	uint progress;
};

struct InterruptCompat {
	InterruptType type;
	uint pid;
};

struct ProcessCompat {
	ProcessCompat()
		: pid(-1),
		  name(nullptr),
		  arrivalTime(-1),
		  doneTime(-1),
		  reqProcessorTime(-1),
		  processorTime(-1),
		  level(0),
		  processorTimeOnLevel(0),
		  state(State::ready),
		  regstate({0, 0}) {}

	uint pid;
	const char* name;
	long arrivalTime;
	long doneTime;
	long reqProcessorTime;
	long processorTime;
	uint level;
	long processorTimeOnLevel;
	State state;
	Registers regstate;
};

struct MachineStateCompat {
	uint8_t numCores;
	uint8_t numIODevices;
	uint clockDelay;
	CPUState* cores;
	DeviceState* ioDevices;
};

struct OSStateCompat {
	uint numProcesses;
	ProcessCompat* processList;
	uint numInterrupts;
	InterruptCompat* interrupts;
	uint numReady;
	ProcessCompat* readyList;
	uint numReentering;
	ProcessCompat* reentryList;
	StepAction* stepAction;
	uint time;
	bool paused;
	uint mlfNumReady[NUM_LEVELS];
	ProcessCompat* mlfReadyLists[NUM_LEVELS];
	uint numRequests;
	IORequest* pendingRequests;
	Syscall* pendingSyscalls;
	ProcessCompat* runningProcesses;
};

extern "C" {
Instruction* exported allocInstructionList(uint instructionCount);
void exported freeInstructionList(Instruction* ptr);
char* exported allocString(unsigned int size);
void exported freeString(char* str);

void exported loadProgram(Instruction* instructionList, uint size, char* name);
uint exported spawn(char* name);
void exported pause();
void exported unpause();
void exported setClockDelay(uint delay);
void exported setSchedulingStrategy(SchedulingStrategy strategy);

MachineStateCompat* exported getMachineState();
OSStateCompat* exported getOSState();
}

#endif