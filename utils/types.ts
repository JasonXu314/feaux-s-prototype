export const NUM_LEVELS = 6;

// eslint-disable-next-line @typescript-eslint/no-unused-vars
export type Ptr<T> = number;

export enum ProcessState {
	READY,
	PROCESSING,
	BLOCKED,
	DONE
}

export enum StepAction {
	NOOP,
	HANDLE_INTERRUPT,
	BEGIN_RUN,
	CONTINUE_RUN,
	HANDLE_SYSCALL,
	SERVICE_REQUEST
}

export enum SchedulingStrategy {
	FIFO,
	SJF,
	SRT,
	MLF
}

export enum Opcode {
	NOP,
	WORK,
	IO,
	EXIT,
	LOAD,
	MOVE,
	ALLOC,
	FREE,
	SW,
	CMP,
	JL,
	JLE,
	JE,
	JGE,
	JG,
	INC,
	ADD
}

export enum InterruptType {
	IO_COMPLETION
}

export enum Syscall {
	SYS_NONE,
	SYS_IO,
	SYS_EXIT
}

export enum Regs {
	RAX,
	RCX,
	RDX,
	RBX,
	RSI,
	RDI,
	RSP,
	RBP,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15
}

export type IOInterrupt = {
	ioEventID: number;
	procID: number;
};

export type IORequest = {
	pid: number;
	duration: number;
};

export type Instruction = {
	opcode: Opcode;
	operand1: number;
	operand2: number;
};

export type Registers = {
	rip: number;
	rax: number;
	rcx: number;
	rdx: number;
	rbx: number;
	rsi: number;
	rdi: number;
	rsp: number;
	rbp: number;
	r8: number;
	r9: number;
	r10: number;
	r11: number;
	r12: number;
	r13: number;
	r14: number;
	r15: number;
};

export const DEF_REGSTATE = {
	rip: -1,
	rax: -1,
	rcx: -1,
	rdx: -1,
	rbx: -1,
	rsi: -1,
	rdi: -1,
	rsp: -1,
	rbp: -1,
	r8: -1,
	r9: -1,
	r10: -1,
	r11: -1,
	r12: -1,
	r13: -1,
	r14: -1,
	r15: -1
};

export type DeviceState = {
	pid: number;
	duration: number;
	progress: number;
};

export type RawProcess = {
	pid: number;
	name: Ptr<string>;
	arrivalTime: number;
	doneTime: number;
	reqProcessorTime: number;
	processorTime: number;
	level: number;
	processorTimeOnLevel: number;
	state: ProcessState;
	regstate: Registers;
};

