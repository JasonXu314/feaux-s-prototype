#ifndef MACHINE_H
#define MACHINE_H

#include "decls.h"
#include "signals.h"

struct CPUState;
struct DeviceState;

class CPU {
public:
	CPU(uint8_t id);

	void load(Registers regState);

	bool free() const;

	Registers regstate() const;

	void tick();

	friend void exportCPU(const CPU& src, CPUState& dest);

private:
	uint8_t _id;
	Instruction* _instruction;
	Registers _registers;

	void _readNextInstruction();
};

class IODevice {
public:
	IODevice(uint8_t id) : _id(id), _pid(0), _duration(0), _progress(0) {}

	void tick();

	void handle(const IORequest& req);

	void clear();

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