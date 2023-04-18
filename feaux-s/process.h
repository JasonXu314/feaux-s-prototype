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
	PCB() : pid(999999), arrivalTime(-1), doneTime(-1), reqProcessorTime(0), processorTime(0), state(ready), level(-1), processorTimeOnLevel(0) {}

	uint pid;  // The process ID, assigned when the process is admitted to the system
	string name;

	long arrivalTime;		// When the process will start/become runnable
	long doneTime;			// Convenience variable, use this to keep track of when a process completes
	long reqProcessorTime;	// Total amount of processor time needed
	long processorTime;		// Amount of processor given to this process
	uint level;
	long processorTimeOnLevel;

	State state;  // State of the process
	Registers regstate;
};
