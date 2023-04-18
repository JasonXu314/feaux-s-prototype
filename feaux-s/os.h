#ifndef OS_H
#define OS_H

#include <queue>

#include "decls.h"
#include "signals.h"

void initOS(uint numCores, SchedulingStrategy strategy);
void cleanupOS();

PCB* schedule(uint core);

void handleInterrupt(Interrupt* interrupt);

#endif