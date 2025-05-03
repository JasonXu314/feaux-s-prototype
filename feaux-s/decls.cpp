#include "decls.h"

#include "process.h"

uint nextPID = 0;

// NOOP register state (see machine.cpp#CPU::_readNextInstruction)
const Registers NOPROC{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

bool SJFComparator::operator()(PCB* a, PCB* b) {
	return a->reqProcessorTime > b->reqProcessorTime;
}

bool SRTComparator::operator()(PCB* a, PCB* b) {
	return (a->reqProcessorTime - a->processorTime) > (b->reqProcessorTime - b->processorTime);
}

bool EDFComparator::operator()(PCB* a, PCB* b) {
	return a->deadline > b->deadline;
}

bool LSTComparator::operator()(PCB* a, PCB* b) {
	return (a->deadline - (a->reqProcessorTime - a->processorTime)) > (b->deadline - (b->reqProcessorTime - b->processorTime));
}