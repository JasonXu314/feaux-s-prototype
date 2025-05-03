#include "utils.h"

#include "browser-api.h"

using namespace std;

void exportProcess(const PCB& src, ProcessCompat& dest) {
	dest.pid = src.pid;
	dest.name = src.name.c_str();
	dest.arrivalTime = src.arrivalTime;
	dest.deadline = src.deadline;
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

uint* getRegister(Registers& regs, Regs reg) {
	switch (reg) {
		case Regs::RAX:
			return &regs.rax;
		case Regs::RCX:
			return &regs.rcx;
		case Regs::RDX:
			return &regs.rdx;
		case Regs::RBX:
			return &regs.rbx;
		case Regs::RSI:
			return &regs.rsi;
		case Regs::RDI:
			return &regs.rdi;
		case Regs::RSP:
			return &regs.rsp;
		case Regs::RBP:
			return &regs.rbp;
		case Regs::R8:
			return &regs.r8;
		case Regs::R9:
			return &regs.r9;
		case Regs::R10:
			return &regs.r10;
		case Regs::R11:
			return &regs.r11;
		case Regs::R12:
			return &regs.r12;
		case Regs::R13:
			return &regs.r13;
		case Regs::R14:
			return &regs.r14;
		case Regs::R15:
			return &regs.r15;
		default:
			cerr << "Unknown register " << reg << endl;
			return nullptr;
	}
}