#include "decls.h"

#include "process.h"

uint nextPID = 0;
MachineState* machineState = nullptr;
OSState* state = nullptr;

bool SJFComparator::operator()(Process* a, Process* b) { return a->reqProcessorTime > b->reqProcessorTime; }

bool SRTComparator::operator()(Process* a, Process* b) { return (a->reqProcessorTime - a->processorTime) > (b->reqProcessorTime - b->processorTime); }