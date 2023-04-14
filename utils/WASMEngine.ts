import { Memory } from './Memory';
import { Process } from './cpp-compat/Process';
import { IOInterrupt, Instruction, MachineState, OSState, Ptr, RawMachineState, RawOSState, SchedulingStrategy } from './types';

export interface WASMModule {
	HEAP8: Int8Array;
	HEAP16: Int16Array;
	HEAP32: Int32Array;
	HEAPF32: Float32Array;
	HEAPF64: Float64Array;
	HEAPU8: Uint8Array;
	HEAPU16: Uint16Array;
	HEAPU32: Uint32Array;
	asm: {
		memory: WebAssembly.Memory;

		addProcess(instructionList: Ptr<Instruction[]>, size: number, stringPtr: Ptr<string>): void;
		allocInstructionList(size: number): Ptr<Instruction[]>;
		allocString(size: number): Ptr<string>;
		freeInstructionList(addr: Ptr<Instruction[]>): void;
		freeString(addr: Ptr<string>): void;
		getMachineState(): Ptr<RawMachineState>;
		getOSState(): Ptr<RawOSState>;
		pause(): void;
		unpause(): void;
		setClockDelay(delay: number): void;
		setSchedulingStrategy(strategy: SchedulingStrategy): void;
	};
}

export class WASMEngine {
	private readonly memory: Memory;

	constructor(private readonly module: WASMModule) {
		this.memory = new Memory(module.asm.memory);
	}

	public addProcess(instructionList: Instruction[], name: string): void {
		const ilPtr = this.module.asm.allocInstructionList(instructionList.length);
		instructionList.forEach((instruction, i) => {
			this.memory.writeUint8(ilPtr + i * 2, instruction.opcode);
			this.memory.writeUint8(ilPtr + i * 2 + 1, instruction.operand);
		});

		const strPtr = this.module.asm.allocString(name.length);
		this.memory.writeString(strPtr, name);

		this.module.asm.addProcess(ilPtr, instructionList.length, strPtr);

		this.module.asm.freeInstructionList(ilPtr);
		this.module.asm.freeString(strPtr);
	}

	public getMachineState(): MachineState {
		const ptr = this.module.asm.getMachineState();

		const numCores = this.memory.readUint32(ptr);
		const clockDelay = this.memory.readUint32(ptr + 4);

		const availablePtr = this.memory.readUint32(ptr + 8);
		const available = new Array(numCores).fill(null).map((_, i) => this.memory.readUint8(availablePtr + i) === 1);

		const runningProcess = Process.readFrom(this.memory, this.memory.readUint32(ptr + 12), numCores);

		return {
			numCores,
			clockDelay,
			available,
			runningProcess
		};
	}

	public getOSState(): OSState {
		const ptr = this.module.asm.getOSState();

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

		const numStepAction = this.getMachineState().numCores;
		const stepActionPtr = this.memory.readUint32(ptr + 32);
		const stepAction = new Array(numStepAction).fill(null).map((_, i) => this.memory.readUint32(stepActionPtr + i * 4));

		const time = this.memory.readUint32(ptr + 36);
		const paused = this.memory.readUint8(ptr + 40) === 1;

		const mlfNumReady = new Array(6).fill(null).map((_, i) => this.memory.readUint32(ptr + 44 + i * 4));
		const mlfReadyLists = new Array(6).fill(null).map((_, i) => Process.readFrom(this.memory, this.memory.readUint32(ptr + 68 + i * 4), mlfNumReady[i]));

		return {
			processList,
			interrupts,
			readyList,
			reentryList,
			stepAction,
			time,
			paused,
			mlfReadyLists
		};
	}

	public pause(): void {
		this.module.asm.pause();
	}

	public unpause(): void {
		this.module.asm.unpause();
	}

	public setClockDelay(delay: number): void {
		this.module.asm.setClockDelay(delay);
	}

	public setSchedulingStrategy(strategy: SchedulingStrategy): void {
		this.module.asm.setSchedulingStrategy(strategy);
	}
}

