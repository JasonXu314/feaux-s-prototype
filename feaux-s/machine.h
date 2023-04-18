#ifndef MACHINE_H
#define MACHINE_H

#include "decls.h"
#include "signals.h"

struct CPUState;
struct DeviceState;

// Class for simulating the operations of a CPU
class CPU {
public:
	CPU(uint8_t id);

	// Loads the registers into the CPU
	void load(Registers regState);

	// Checks whether the CPU is currently free (not executing a process)
	bool free() const;

	// Gets the current register state of the CPU (ie. before removing a process from execution)
	Registers regstate() const;

	// Runs a tick of the simulation
	void tick();

	friend void exportCPU(const CPU& src, CPUState& dest);

private:
	uint8_t _id;
	Instruction* _instruction;
	Registers _registers;

	void _readNextInstruction();
};

// Class for simulating the operations of an I/O device
class IODevice {
public:
	IODevice(uint8_t id) : _id(id), _pid(0), _duration(0), _progress(0) {}

	// Runs a tick of the simulation
	void tick();

	// Informs the I/O device of the request and starts processing
	void handle(const IORequest& req);

	// Resets the state of the I/O device to not processing
	void clear();

	// Checks whether the I/O device is currently free
	bool busy() const { return _pid != 0; }

	friend void exportIODevice(const IODevice& src, DeviceState& dest);

private:
	uint8_t _id;
	uint _pid;
	uint _duration;
	uint _progress;
};

void initMachine(uint8_t numCores, uint8_t numIODevices);
void cleanupMachine();

#endif