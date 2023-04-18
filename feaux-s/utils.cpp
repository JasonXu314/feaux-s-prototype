#include "utils.h"

#include "browser-api.h"

using namespace std;

void exportProcess(const PCB& src, ProcessCompat& dest) {
	dest.pid = src.pid;
	dest.name = src.name.c_str();
	dest.arrivalTime = src.arrivalTime;
	dest.doneTime = src.doneTime;
	dest.reqProcessorTime = src.reqProcessorTime;
	dest.processorTime = src.processorTime;
	dest.level = src.level;
	dest.processorTimeOnLevel = src.processorTimeOnLevel;
	dest.state = src.state;
	dest.regstate = src.regstate;
}

void exportCPU(const CPU& src, CPUState& dest) {
	dest.available = src.free();
	dest.regstate = src._registers;
}

void exportIODevice(const IODevice& src, DeviceState& dest) {
	dest.pid = src._pid;
	dest.duration = src._duration;
	dest.progress = src._progress;
}

void exportInterrupt(const Interrupt& src, InterruptCompat& dest) {
	dest.type = src.type();

	switch (src.type()) {
		case InterruptType::IO_COMPLETION:
			const IOInterrupt& io = (IOInterrupt&)src;
			dest.pid = io.pid();
			break;
	}
}