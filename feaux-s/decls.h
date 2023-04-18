#ifndef DECLS_H
#define DECLS_H

#include <emscripten.h>

#include <list>
#include <map>
#include <queue>
#include <string>

struct PCB;
struct IOInterrupt;
class Interrupt;
class CPU;
class IODevice;

#define exported EMSCRIPTEN_KEEPALIVE
#define NUM_LEVELS 6
typedef unsigned int uint;

enum StepAction { NOOP, HANDLE_INTERRUPT, BEGIN_RUN, CONTINUE_RUN, HANDLE_SYSCALL };
enum SchedulingStrategy { FIFO, SJF, SRT, MLF };
enum State { ready, processing, blocked, done };  // Used to track the process states
enum Opcode { NOP, WORK, IO, EXIT };
enum InterruptType { IO_COMPLETION };
enum Syscall { SYS_NONE, SYS_IO, SYS_EXIT };

struct Instruction {
	uint8_t opcode;
	uint8_t operand;
};

struct Program {
	Program() : name(""), length(0), instructions(nullptr) {}
	Program(const std::string& name, uint length, Instruction* instructions) : name(name), length(length), instructions(instructions) {}
	Program(const Program& other) : name(other.name), length(other.length), instructions(new Instruction[other.length]) {
		for (uint i = 0; i < length; i++) {
			instructions[i] = other.instructions[i];
		}
	}

	std::string name;
	uint length;
	Instruction* instructions;

	bool operator<(const Program& other) { return name < other.name; }

	~Program() { delete[] instructions; }
};

struct Registers {
	// Pointer to next instruction
	uint rip;

	// Argument registers
	uint rdi;
};

struct IORequest {
	uint pid;
	uint duration;
};

struct MachineState {
	uint8_t numCores;
	uint8_t numIODevices;
	uint clockDelay;
	CPU** cores;
	IODevice** ioDevices;
};

class SJFComparator {
public:
	bool operator()(PCB* a, PCB* b);
};

class SRTComparator {
public:
	bool operator()(PCB* a, PCB* b);
};

struct OSState {
	std::list<PCB*> processList;
	std::list<Interrupt*> interrupts;
	std::queue<PCB*> fifoReadyList;
	std::priority_queue<PCB*, std::vector<PCB*>, SJFComparator> sjfReadyList;
	std::priority_queue<PCB*, std::vector<PCB*>, SRTComparator> srtReadyList;
	std::queue<PCB*>* mlfLists;
	std::list<PCB*> reentryList;
	std::queue<IORequest> pendingRequests;
	StepAction* stepAction;
	Syscall* pendingSyscalls;
	PCB** runningProcess;
	uint time;
	bool paused;
	SchedulingStrategy strategy;
	std::map<std::string, Program> programs;
};

extern MachineState* machine;
extern OSState* state;
extern uint nextPID;
extern const Registers NOPROC;

#endif