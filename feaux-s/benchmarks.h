#ifndef BENCHMARKS_H
#define BENCHMARKS_H

#include <cmath>

#include "browser-api.h"
#include "decls.h"

#define STRATEGY_NAME(strategy)                                        \
	(strategy == SchedulingStrategy::FIFO  ? "First-In-First-Out"      \
	 : strategy == SchedulingStrategy::SJF ? "Shortest Job First"      \
	 : strategy == SchedulingStrategy::SRT ? "Shortest Remaining Time" \
	 : strategy == SchedulingStrategy::MLF ? "Multi-Level Feedback"    \
										   : "oops...")

void printStats();
bool simulate();

#endif