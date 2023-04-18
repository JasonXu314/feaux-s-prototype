#include "machine.h"

#include <iostream>

#include "os.h"

using namespace std;

MachineState* machine = nullptr;

CPU::CPU(uint8_t id) : _id(id) {
	_instruction = nullptr;
	_registers.rip = (uint) nullptr;
}

bool CPU::free() const { return _registers.rip == 0; }

Registers CPU::regstate() const { return _registers; }

void CPU::load(Registers regState) { _registers = regState; }

void CPU::tick() {
	_readNextInstruction();

	if (_instruction != nullptr) {
		switch (_instruction->opcode) {
			case Opcode::NOP:
			case Opcode::WORK:
				break;
			case Opcode::IO:
				state->pendingSyscalls[_id] = Syscall::SYS_IO;
				_registers.rdi = _instruction->operand;
				break;
			case Opcode::EXIT:
				state->pendingSyscalls[_id] = Syscall::SYS_EXIT;
				break;
		}
	}
}

void CPU::_readNextInstruction() {
	if (_registers.rip == 0) {
		_instruction = nullptr;
	} else {
		_instruction = ((Instruction*)_registers.rip);
		_registers.rip += sizeof(Instruction);
	}
}

void IODevice::tick() {
	if (_pid != 0) {
		_progress++;

		if (_progress >= _duration) {
			IOInterrupt* interrupt = new IOInterrupt(_pid);

			handleInterrupt(interrupt);
			clear();
		}
	}
}

void IODevice::handle(const IORequest& req) {
	if (_pid != 0) {
		cerr << "IO Device " << _id << " asked to handle request from process " << req.pid << " while busy" << endl;
	}

	_pid = req.pid;
	_duration = req.duration;
	_progress = 0;
}

void IODevice::clear() {
	_pid = 0;
	_duration = 0;
	_progress = 0;
}

void initMachine(uint8_t numCores, uint8_t numIODevices) {
	machine = new MachineState{numCores, numIODevices, 500};

	machine->cores = new CPU*[numCores];
	for (uint8_t i = 0; i < numCores; i++) {
		machine->cores[i] = new CPU(i);
	}

	machine->ioDevices = new IODevice*[numIODevices];
	for (uint8_t i = 0; i < numIODevices; i++) {
		machine->ioDevices[i] = new IODevice(i);
	}
}

void cleanupMachine() {
	for (uint8_t i = 0; i < machine->numCores; i++) {
		delete machine->cores[i];
	}
	delete[] machine->cores;

	for (uint8_t i = 0; i < machine->numIODevices; i++) {
		delete machine->ioDevices[i];
	}
	delete[] machine->ioDevices;
}