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

// The OS action at the current step of the simulation
enum StepAction { NOOP, HANDLE_INTERRUPT, BEGIN_RUN, CONTINUE_RUN, HANDLE_SYSCALL, SERVICE_REQUEST };
// The scheduling strategies
// FIFO = First In, First Out
// SJF = Shortest Job First
// SRT = Shortest Remaining Time
// MLF = Multi-Level Feedback
enum SchedulingStrategy { FIFO, SJF, SRT, MLF };
// The states a process can be in
enum State { ready, processing, blocked, done };
// The opcodes for CPU instructions
enum Opcode { NOP, WORK, IO, EXIT, LOAD, MOVE, ALLOC, FREE, SW, CMP, JL, JLE, JE, JGE, JG, INC, ADD };
// The available x86-64 registers (yes i know in my imiplementation they're 32-bit, not 64-bit,
// but WASM interacts weirdly with unsigned long longs for some reason)
enum Regs { RAX, RCX, RDX, RBX, RSI, RDI, RSP, RBP, R8, R9, R10, R11, R12, R13, R14, R15 };
// The types of interrupt that can occur
enum InterruptType { IO_COMPLETION };
// The syscalls available to processes
enum Syscall { SYS_NONE, SYS_IO, SYS_EXIT, SYS_ALLOC, SYS_FREE };

// A CPU instruction
struct Instruction {
	Opcode opcode;
	uint operand1;
	uint operand2;
};

// A program that is recognized by the OS
struct Program {
	// Makes a "blank" program
	Program() : name(""), length(0), instructions(nullptr) {}
	// Constructs a program from the given data
	Program(const std::string& name, uint length, Instruction* instructions) : name(name), length(length), instructions(instructions) {}
	// Copies another program
	Program(const Program& other) : name(other.name), length(other.length), instructions(new Instruction[other.length]) {
		for (uint i = 0; i < length; i++) {
			instructions[i] = other.instructions[i];
		}
	}

	// The name of the program (will be shared by processes executing this program's instructions)
	std::string name;
	// The size of the program
	uint length;
	Instruction* instructions;

	// Necessary to store programs in a map (i think, actually maybe not but im not going to remove it because :P)
	bool operator<(const Program& other) { return name < other.name; }

	// Free a program
	~Program() { delete[] instructions; }
};

#define FLAG_CY 0x0001
#define FLAG_ZF 0x0040

// The current register state (of a process or CPU)
struct Registers {
	// Status registers
	uint rip;	 // Instruction pointer
	uint flags;	 // Flags register (https://en.wikipedia.org/wiki/FLAGS_register) (yes, i know its only 32 bit because uint)

	// GPRs
	uint rax;  // %rax
	uint rcx;  // %rcx
	uint rdx;  // %rdx
	uint rbx;  // %rbx
	uint rsi;  // %rsi
	uint rdi;  // %rdi
	uint rsp;  // %rsp
	uint rbp;  // %rbp
	uint r8;   // %r8
	uint r9;   // %r9
	uint r10;  // %r10
	uint r11;  // %r11
	uint r12;  // %r12
	uint r13;  // %r13
	uint r14;  // %r14
	uint r15;  // %r15
};

// An I/O request made by a process
struct IORequest {
	uint pid;
	uint duration;
};

// The current state of the simulation machine
struct MachineState {
	uint8_t numCores;
	uint8_t numIODevices;
	uint clockDelay;
	CPU** cores;		   // note: these are not 2-d arrays, just arrays of pointers (so that i can use nullptr)
	IODevice** ioDevices;  // note: these are not 2-d arrays, just arrays of pointers (so that i can use nullptr)
};

// A class for the SJF (Shortest Job First) priority queue to be able to compare 2 processes
class SJFComparator {
public:
	bool operator()(PCB* a, PCB* b);
};

// A class for the SRT (Shortest Remaining Time) priority queue to be able to compare 2 processes
class SRTComparator {
public:
	bool operator()(PCB* a, PCB* b);
};

// The data kept track of by the OS
struct OSState {
	std::list<PCB*> processList;											   // A list of all the processes that have/are/will execute
	std::list<Interrupt*> interrupts;										   // A list of the interrupts that the OS has yet to handle
	std::queue<PCB*> fifoReadyList;											   // The ready list for the FIFO scheduling algorithm
	std::priority_queue<PCB*, std::vector<PCB*>, SJFComparator> sjfReadyList;  // The ready list for the SJF scheduling algorithm
	std::priority_queue<PCB*, std::vector<PCB*>, SRTComparator> srtReadyList;  // The ready list for the SRT scheduling algorithm
	std::queue<PCB*>* mlfLists;												   // The ready lists for the MLF scheduling algorithm (will always be 6 long)
	std::list<PCB*> reentryList;											   // The list of processes that, on this cycle, had I/O operations complete
	std::queue<IORequest> pendingRequests;	// The pending I/O requests (raised by a process, but all I/O Devices were busy)
	StepAction* stepAction;					// The current action for each core at this step of the simulation
	Syscall* pendingSyscalls;				// The pending syscalls for each core
	PCB** runningProcess;					// The currently running process for each core
	uint time;
	bool paused;
	SchedulingStrategy strategy;
	std::map<std::string, Program> programs;  // The set of all programs known to the OS
};

// Some declarations for global state
extern MachineState* machine;
extern OSState* state;
extern uint nextPID;
extern const Registers NOPROC;

#endif