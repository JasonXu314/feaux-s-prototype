#ifndef UTILS_H
#define UTILS_H

#include <emscripten.h>

#include <list>
#include <queue>

#include "decls.h"
#include "ioModule.h"
#include "process.h"

void exportProcess(const Process& src, ProcessCompat& dest);

#endif