/*

Relationship between IOEvent::time and Process time:

IOEvent::time represents the number of time steps into a process execution that
an IO Event will occur. This value will be relative to Process::processorTime.

For example, if a Process has an IOEvent with a time value of 10, then that means
that when the process hits it's 10th time step (i.e., Process::processorTime = 10)
it should enter a blocked state for IOEvent::duration time steps.

*/

#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <vector>

#include "decls.h"

using namespace std;

struct PCB {
	PCB()
		: pid(999999),
		  arrivalTime(-1),
		  doneTime(-1),
		  reqProcessorTime(0),
		  processorTime(0),
		  level(-1),
		  processorTimeOnLevel(0),
		  state(ready),
		  regstate(NOPROC) {}

	uint pid;					// The process ID, assigned when the process is admitted to the system
	string name;				// The name of the process (same as program name)
	long arrivalTime;			// When the process was spawned
	long doneTime;				// The time that the process completed execution
	long reqProcessorTime;		// Total amount of processor time needed (number of instructions)
	long processorTime;			// Total amount of processor time this process has received
	uint level;					// The level the process is on (for MLF processing)
	long processorTimeOnLevel;	// The amount of CPU time the process has received on the current level (for MLF processing)
	State state;				// State of the process
	Registers regstate;			// The saved state of registers of the process
};
