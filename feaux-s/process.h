/*

Relationship between IOEvent::time and Process time:

IOEvent::time represents the number of time steps into a process execution that
an IO Event will occur. This value will be relative to Process::processorTime.

For example, if a Process has an IOEvent with a time value of 10, then that means
that when the process hits it's 10th time step (i.e., Process::processorTime = 10)
it should enter a blocked state for IOEvent::duration time steps.

*/

#pragma once

#include <emscripten.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <vector>

using namespace std;
typedef unsigned int uint;

struct IOEvent {
	IOEvent() : id(9999999), time(-1), duration(0){};
	IOEvent(const int& t, const int& d, const unsigned int& newId) : id(newId), time(t), duration(d) {}

	unsigned int id;

	long time;		// The time the event occurs during the process execution
	long duration;	// The duration that the process will be Blocked by this IOEvent
};

enum State { ready, processing, blocked, done };  // Used to track the process states

struct Process {
	Process() : id(999999), arrivalTime(-1), doneTime(-1), reqProcessorTime(0), processorTime(0), state(ready) {}

	unsigned int id;  // The process ID, assigned when the process is admitted to the system
	string name;

	long arrivalTime;		// When the process will start/become runnable
	long doneTime;			// Convenience variable, use this to keep track of when a process completes
	long reqProcessorTime;	// Total amount of processor time needed
	long processorTime;		// Amount of processor given to this process

	State state;  // State of the process

	list<IOEvent> ioEvents;	 // The IO events for this process, stored in order of the time into the process execution that they start
};

enum Opcode { NOP, WORK, IO };

struct Instruction {
	uint8_t opcode;
	uint8_t operand;
};

extern "C" {
// use EMSCRIPTEN_KEEPALIVE because including utils.h would introduce circular dependencies
Instruction* EMSCRIPTEN_KEEPALIVE allocInstructionList(uint instructionCount);
void EMSCRIPTEN_KEEPALIVE freeInstructionList(Instruction* ptr);
}