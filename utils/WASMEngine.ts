import { Memory } from './Memory';
import { IOEvent, Instruction, MachineState, Process, Ptr, RawMachineState, RawOSState } from './types';

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
	};
}

export class WASMEngine {
	private readonly memory: Memory;

	constructor(private readonly module: WASMModule) {
		console.log(module);
		this.memory = new Memory(module.asm.memory);
	}

	public getMachineState(): MachineState {
		const ptr = this.module.asm.getMachineState();

		const numCores = this.memory.getUint32(ptr);
		const clockDelay = this.memory.getUint32(ptr + 4);

		const availablePtr = this.memory.getUint32(ptr + 8);
		const available = new Array(4).fill(null).map((_, i) => this.memory.getUint8(availablePtr + i) === 1);

		const runningProcessPtr = this.memory.getUint32(ptr + 12);
		const runningProcess = new Array(4).fill(null).map((_, i) => this.readProcess(runningProcessPtr + i * 36));

		return {
			numCores,
			clockDelay,
			available,
			runningProcess
		};
	}

	private readString(ptr: Ptr<string>): string {
		let str = '',
			i = 0,
			codePoint = -1;

		while ((codePoint = this.memory.getUint8(ptr + i)) !== 0) {
			str += String.fromCodePoint(codePoint);
			i++;
		}

		return str;
	}

	private readProcess(ptr: Ptr<Process>): Process {
		const id = this.memory.getUint32(ptr);
		const name = this.readString(this.memory.getUint32(ptr + 4));
		const arrivalTime = this.memory.getInt32(ptr + 8);
		const doneTime = this.memory.getInt32(ptr + 12);
		const reqProcessorTime = this.memory.getInt32(ptr + 16);
		const processorTime = this.memory.getInt32(ptr + 20);
		const state = this.memory.getUint32(ptr + 24);
		const numIOEvents = this.memory.getUint32(ptr + 28);
		const ioEventPtr = this.memory.getUint32(ptr + 32);

		const ioEvents = new Array(numIOEvents).fill(null).map<IOEvent>((_, i) => ({
			id: this.memory.getUint32(ioEventPtr + i * 12),
			time: this.memory.getInt32(ioEventPtr + i * 12 + 4),
			duration: this.memory.getInt32(ioEventPtr + i * 12 + 8)
		}));

		return {
			id,
			name,
			arrivalTime,
			doneTime,
			reqProcessorTime,
			processorTime,
			state,
			ioEvents
		};
	}
}

