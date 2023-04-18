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

void exportProcess(const PCB& src, ProcessCompat& dest);

void exportCPU(const CPU& src, CPUState& dest);

void exportIODevice(const IODevice& src, DeviceState& dest);

void exportInterrupt(const Interrupt& src, InterruptCompat& dest);

#endif