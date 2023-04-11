#include "process.h"

Instruction* EMSCRIPTEN_KEEPALIVE allocInstructionList(uint instructionCount) { return new Instruction[instructionCount]; }

void EMSCRIPTEN_KEEPALIVE freeInstructionList(Instruction* ptr) { delete[] ptr; }