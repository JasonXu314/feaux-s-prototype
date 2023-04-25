#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "decls.h"

// A general interrupt (could be I/O interrupt or other like network or something)
class Interrupt {
public:
	Interrupt(InterruptType type) : _type(type) {}

	InterruptType type() const;

protected:
	InterruptType _type;
};

// An I/O Interrupt (signals completion of an I/O operation)
class IOInterrupt : public Interrupt {
public:
	IOInterrupt(uint pid) : Interrupt(InterruptType::IO_COMPLETION), _pid(pid) {}

	// Gets the PID of the process for which the I/O operation completed
	uint pid() const { return _pid; }

private:
	uint _pid;
};

#endif