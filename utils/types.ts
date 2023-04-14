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
	IO_REQUEST,
	COMPLETE
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
	IO
}

export type IOEvent = {
	id: number;
	time: number;
	duration: number;
};

export type IOInterrupt = {
	ioEventID: number;
	procID: number;
};

export type Instruction = {
	opcode: Opcode;
	operand: number;
};

export type Process = {
	id: number;
	name: string;
	arrivalTime: number;
	doneTime: number;
	reqProcessorTime: number;
	processorTime: number;
	state: ProcessState;
	ioEvents: IOEvent[];
};

export type RawProcess = {
	id: number;
	name: Ptr<string>;
	arrivalTime: number;
	doneTime: number;
	reqProcessorTime: number;
	processorTime: number;
	state: ProcessState;
	ioEvents: Ptr<IOEvent>;
};

export type MachineState = {
	numCores: number;
	clockDelay: number;
	available: boolean[];
	runningProcess: Process[];
};

export type RawMachineState = {
	numCores: number;
	clockDelay: number;
	available: boolean[];
	runningProcess: Ptr<Process>[];
};

export type OSState = {
	processList: Process[];
	interrupts: IOInterrupt[];
	readyList: Process[];
	reentryList: Process[];
	stepAction: StepAction[];
	time: number;
	paused: boolean;
};

export type RawOSState = {
	processList: Ptr<Process>;
	interrupts: Ptr<IOInterrupt>;
	readyList: Ptr<Process>;
	reentryList: Ptr<Process>;
	stepAction: Ptr<StepAction>;
	time: number;
	paused: boolean;
};

