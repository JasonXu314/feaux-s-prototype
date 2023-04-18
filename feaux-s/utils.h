#ifndef UTILS_H
#define UTILS_H

#include <emscripten.h>

#include <list>
#include <queue>

#include "decls.h"
#include "machine.h"
#include "process.h"

struct ProcessCompat;
struct InterruptCompat;

// Writes the data from the PCB into a format that the compatibility layer will recognize
void exportProcess(const PCB& src, ProcessCompat& dest);

// Writes the current CPU state into a format that the compatibility layer will recognize
void exportCPU(const CPU& src, CPUState& dest);

// Writes the current I/O Device state into a format that the compatibility layer will recognize
void exportIODevice(const IODevice& src, DeviceState& dest);

// Writes the data of the interrupt into a format that the compatibility layer will recognize
void exportInterrupt(const Interrupt& src, InterruptCompat& dest);

// Gets the address of the register (convenience function for direct writing in CPU operations)
uint* getRegister(Registers& regs, Regs reg);

#endif