#ifndef BROWSER_API_H
#define BROWSER_API_H

#include <string.h>

#include "decls.h"
#include "utils.h"

// Declarations for OS setup/teardown functions (used for simulation parameter adjustment)
extern void initOS(uint numCores, SchedulingStrategy strategy);
extern void cleanupOS();

// The current state of a CPU (for compatibility layer)
struct CPUState {
	bool available;
	Registers regstate;
};

// The current state of an I/O Device (for compatibility layer)
struct DeviceState {
	uint pid;
	uint duration;
	uint progress;
};

// A data format for interrupts that the compatibility layer will recognize
struct InterruptCompat {
	InterruptType type;
	uint pid;
};

// A data format for PCBs that the compatibility layer will recognize
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

// A data format for the simulated machine that the compatibility layer will recognize
struct MachineStateCompat {
	uint8_t numCores;
	uint8_t numIODevices;
	uint clockDelay;
	CPUState* cores;
	DeviceState* ioDevices;
};

// A data format for the OS that the compatibility layer will recognize
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
// Allocates data for an instruction list (for program) of the given size
Instruction* exported allocInstructionList(uint instructionCount);

// Fress data previously allocated for an instruction list
void exported freeInstructionList(Instruction* ptr);

// Allocates data for a string of the given size (will pad with '\0' at the end for c string semantics, so allocates 1 additional byte)
char* exported allocString(unsigned int size);

// Fress data previously allocated for a string
void exported freeString(char* str);

// Loads a program into the OS
// Note that the instruction list and name should have been earlier alloc'd and written to, and should be later freed (by the caller)
// This function does not free the instruction list/name
void exported loadProgram(Instruction* instructionList, uint size, char* name);

// Gets the memory address where a program's instructions are stored (for monitoring progress since loops are a thing)
Instruction* exported getProgramLocation(char* name);

// Spawns a process with the program specified by the given name
uint exported spawn(char* name);

// Pause the simulation
void exported pause();

// Unause the simulation
void exported unpause();

// Set the clock delay of the simulated machine (in ms) (ie. the time between ticks)
void exported setClockDelay(uint delay);

void exported setNumCores(uint8_t cores);
void exported setNumIODevices(uint8_t ioDevices);

// Set the scheduling strategy of the OS
// Needs to reboot OS, so will lose all processes (but keeps programs)
void exported setSchedulingStrategy(SchedulingStrategy strategy);

// Get the current state of the machine
MachineStateCompat* exported getMachineState();

// Get the current state of the OS
OSStateCompat* exported getOSState();
}

#endif