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
	HANDLE_SYSCALL
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
	EXIT
}

export enum InterruptType {
	IO_COMPLETION
}

export enum Syscall {
	SYS_NONE,
	SYS_IO,
	SYS_EXIT
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
	operand: number;
};

export type Registers = {
	rip: number;
	rdi: number;
};

export type Process = {
	pid: number;
	name: string;
	arrivalTime: number;
	doneTime: number;
	reqProcessorTime: number;
	processorTime: number;
	state: ProcessState;
};

export type CPUState = {
	available: boolean;
	regstate: Registers;
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

export type MachineState = {
	numCores: number;
	numIODevices: number;
	clockDelay: number;
	cores: CPUState[];
	ioDevices: DeviceState[];
};

export type OSState = {
	processList: Process[];
	interrupts: IOInterrupt[];
	readyList: Process[];
	reentryList: Process[];
	stepAction: StepAction[];
	time: number;
	paused: boolean;
	mlfReadyLists: Process[][];
	pendingRequests: IORequest[];
	pendingSyscalls: Syscall[];
	runningProcesses: Process[];
};

