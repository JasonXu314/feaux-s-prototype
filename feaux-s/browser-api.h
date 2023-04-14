#ifndef BROWSER_API_H
#define BROWSER_API_H

#include "decls.h"
#include "utils.h"

extern void initOS(uint numCores, SchedulingStrategy strategy);
extern void cleanupOS();

extern "C" {
Instruction* exported allocInstructionList(uint instructionCount);
void exported freeInstructionList(Instruction* ptr);
char* exported allocString(unsigned int size);
void exported freeString(char* str);

uint exported addProcess(Instruction* instructionList, uint size, char* processName);
void exported pause();
void exported unpause();
void exported setClockDelay(uint delay);
void exported setSchedulingStrategy(SchedulingStrategy strategy);

MachineStateCompat* exported getMachineState();
OSStateCompat* exported getOSState();
}

#endif