#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "decls.h"

class Interrupt {
public:
	Interrupt(InterruptType type) : _type(type) {}

	InterruptType type() const;

protected:
	InterruptType _type;
};

class IOInterrupt : public Interrupt {
public:
	IOInterrupt(uint pid) : Interrupt(InterruptType::IO_COMPLETION), _pid(pid) {}

	uint pid() const { return _pid; }

private:
	uint _pid;
};

#endif