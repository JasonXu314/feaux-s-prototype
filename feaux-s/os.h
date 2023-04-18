#ifndef OS_H
#define OS_H

#include <queue>

#include "decls.h"
#include "signals.h"

// Initializes the OS state according to the given machine specifications and scheduling strategy
void initOS(uint numCores, SchedulingStrategy strategy);
// Clearns up the OS (deallocates memory and stuff)
void cleanupOS();

// Picks a process to execute next according to the OS scheduling strategy
PCB* schedule(uint core);

// Informs the OS that an interrupt has occured
void handleInterrupt(Interrupt* interrupt);

#endif