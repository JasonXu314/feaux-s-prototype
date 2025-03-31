import { Memory } from './Memory';
import { CPUState } from './cpp-compat/CPUState';
import { DeviceState } from './cpp-compat/DeviceState';
import { IORequest } from './cpp-compat/IORequest';
import { Process } from './cpp-compat/Process';
import { IOInterrupt, Instruction, Opcode, Ptr, SchedulingStrategy, StepAction, Syscall } from './types';

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

export type MachineState = {
	numCores: number;
	numIODevices: number;
	clockDelay: number;
	cores: CPUState[];
	ioDevices: DeviceState[];
};

export interface WASMModule {
	HEAP8: Int8Array;
	HEAP16: Int16Array;
	HEAP32: Int32Array;
	HEAPF32: Float32Array;
	HEAPF64: Float64Array;
	HEAPU8: Uint8Array;
	HEAPU16: Uint16Array;
	HEAPU32: Uint32Array;
	wasmExports: {
		memory: WebAssembly.Memory;

		loadProgram(instructionList: Ptr<Instruction[]>, size: number, name: Ptr<string>): void;
		getProgramLocation(name: number): number;
		spawn(name: Ptr<string>): number;
		allocInstructionList(size: number): Ptr<Instruction[]>;
		allocString(size: number): Ptr<string>;
		freeInstructionList(addr: Ptr<Instruction[]>): void;
		freeString(addr: Ptr<string>): void;
		getMachineState(): number;
		getOSState(): number;
		pause(): void;
		unpause(): void;
		setClockDelay(delay: number): void;
		setSchedulingStrategy(strategy: SchedulingStrategy): void;
		setNumCores(cores: number): void;
		setNumIODevices(ioDevices: number): void;
	};
}

export class WASMEngine {
	private readonly memory: Memory;

	constructor(private readonly module: WASMModule) {
		this.memory = new Memory(module.wasmExports.memory);
	}

	public loadProgram(instructionList: Instruction[], name: string): void {
		const ilPtr = this.module.wasmExports.allocInstructionList(instructionList.length);
		instructionList.forEach((instruction, i) => {
			this.memory.writeUint32(ilPtr + i * 12, instruction.opcode);

			if (instruction.opcode === Opcode.JL) {
				this.memory.writeInt32(ilPtr + i * 12 + 4, instruction.operand1);
			} else {
				this.memory.writeUint32(ilPtr + i * 12 + 4, instruction.operand1);
			}

			this.memory.writeUint32(ilPtr + i * 12 + 8, instruction.operand2);
		});

		const strPtr = this.module.wasmExports.allocString(name.length);
		this.memory.writeString(strPtr, name);

		this.module.wasmExports.loadProgram(ilPtr, instructionList.length, strPtr);

		this.module.wasmExports.freeInstructionList(ilPtr);
		this.module.wasmExports.freeString(strPtr);
	}

	public spawn(name: string): number {
		const strPtr = this.module.wasmExports.allocString(name.length);
		this.memory.writeString(strPtr, name);

		const pid = this.module.wasmExports.spawn(strPtr);

		this.module.wasmExports.freeString(strPtr);

		return pid;
	}

	public getMachineState(): MachineState {
		const ptr = this.module.wasmExports.getMachineState();

		const numCores = this.memory.readUint8(ptr);
		const numIODevices = this.memory.readUint8(ptr + 1);
		const clockDelay = this.memory.readUint32(ptr + 4);

		const cores = CPUState.readFrom(this.memory, this.memory.readUint32(ptr + 8), numCores);

		const ioDevices = DeviceState.readFrom(this.memory, this.memory.readUint32(ptr + 12), numIODevices);

		// console.log(numCores, numIODevices, cores, ioDevices);

		return {
			numCores,
			numIODevices,
			clockDelay,
			cores,
			ioDevices
		};
	}

	public getOSState(): OSState {
		const ptr = this.module.wasmExports.getOSState();

		const numProcesses = this.memory.readUint32(ptr);
		const processList = Process.readFrom(this.memory, this.memory.readUint32(ptr + 4), numProcesses);

		const numInterrupts = this.memory.readUint32(ptr + 8);
		const interruptsPtr = this.memory.readUint32(ptr + 12);
		const interrupts = new Array(numInterrupts).fill(null).map<IOInterrupt>((_, i) => ({
			ioEventID: this.memory.readUint32(interruptsPtr + i * 8),
			procID: this.memory.readUint32(interruptsPtr + i * 8 + 4)
		}));

		const numReady = this.memory.readUint32(ptr + 16);
		const readyList = Process.readFrom(this.memory, this.memory.readUint32(ptr + 20), numReady);

		const numReentering = this.memory.readUint32(ptr + 24);
		const reentryList = Process.readFrom(this.memory, this.memory.readUint32(ptr + 28), numReentering);

		const numCores = this.getMachineState().numCores;
		const stepActionPtr = this.memory.readUint32(ptr + 32);
		const stepAction = new Array(numCores).fill(null).map((_, i) => this.memory.readUint32(stepActionPtr + i * 4));

		const time = this.memory.readUint32(ptr + 36);
		const paused = this.memory.readUint8(ptr + 40) === 1;

		const mlfNumReady = new Array(6).fill(null).map((_, i) => this.memory.readUint32(ptr + 44 + i * 4));
		const mlfReadyLists = new Array(6).fill(null).map((_, i) => Process.readFrom(this.memory, this.memory.readUint32(ptr + 68 + i * 4), mlfNumReady[i]));

		const numRequests = this.memory.readUint32(ptr + 92);
		const pendingRequests = IORequest.readFrom(this.memory, this.memory.readUint32(ptr + 96), numRequests);

		const syscallsPtr = this.memory.readUint32(ptr + 100);
		const pendingSyscalls = new Array(numCores).fill(null).map((_, i) => this.memory.readUint32(syscallsPtr + i * 4));

		const runningProcesses = Process.readFrom(this.memory, this.memory.readUint32(ptr + 104), numCores);

		return {
			processList,
			interrupts,
			readyList,
			reentryList,
			stepAction,
			time,
			paused,
			mlfReadyLists,
			pendingRequests,
			pendingSyscalls,
			runningProcesses
		};
	}

	public pause(): void {
		this.module.wasmExports.pause();
	}

	public unpause(): void {
		this.module.wasmExports.unpause();
	}

	public setClockDelay(delay: number): void {
		this.module.wasmExports.setClockDelay(delay);
	}

	public setSchedulingStrategy(strategy: SchedulingStrategy): void {
		this.module.wasmExports.setSchedulingStrategy(strategy);
	}

	public setNumCores(cores: number): void {
		this.module.wasmExports.setNumCores(cores);
	}

	public setNumIODevices(ioDevices: number): void {
		this.module.wasmExports.setNumIODevices(ioDevices);
	}

	public getProgramStart(name: string): number {
		const strPtr = this.module.wasmExports.allocString(name.length);
		this.memory.writeString(strPtr, name);

		const result = this.module.wasmExports.getProgramLocation(strPtr);

		this.module.wasmExports.freeString(strPtr);

		return result;
	}
}

