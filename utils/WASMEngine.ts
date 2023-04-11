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
		this.memory = new Memory(module.asm.memory);
	}

	public addProcess(instructionList: Instruction[], name: string): void {
		const ilPtr = this.module.asm.allocInstructionList(instructionList.length);
		instructionList.forEach((instruction, i) => {
			this.memory.writeUint8(ilPtr + i * 2, instruction.opcode);
			this.memory.writeUint8(ilPtr + i * 2 + 1, instruction.operand);
		});

		const strPtr = this.module.asm.allocString(name.length);
		for (let i = 0; i < name.length; i++) {
			this.memory.writeUint8(strPtr + i, name.codePointAt(i)!);
		}

		this.module.asm.addProcess(ilPtr, instructionList.length, strPtr);

		this.module.asm.freeInstructionList(ilPtr);
		this.module.asm.freeString(strPtr);
	}

	public getMachineState(): MachineState {
		const ptr = this.module.asm.getMachineState();

		const numCores = this.memory.readUint32(ptr);
		const clockDelay = this.memory.readUint32(ptr + 4);

		const availablePtr = this.memory.readUint32(ptr + 8);
		const available = new Array(4).fill(null).map((_, i) => this.memory.readUint8(availablePtr + i) === 1);

		const runningProcessPtr = this.memory.readUint32(ptr + 12);
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

		while ((codePoint = this.memory.readUint8(ptr + i)) !== 0) {
			str += String.fromCodePoint(codePoint);
			i++;
		}

		return str;
	}

	private readProcess(ptr: Ptr<Process>): Process {
		const id = this.memory.readUint32(ptr);
		const name = this.readString(this.memory.readUint32(ptr + 4));
		const arrivalTime = this.memory.readInt32(ptr + 8);
		const doneTime = this.memory.readInt32(ptr + 12);
		const reqProcessorTime = this.memory.readInt32(ptr + 16);
		const processorTime = this.memory.readInt32(ptr + 20);
		const state = this.memory.readUint32(ptr + 24);
		const numIOEvents = this.memory.readUint32(ptr + 28);
		const ioEventPtr = this.memory.readUint32(ptr + 32);

		const ioEvents = new Array(numIOEvents).fill(null).map<IOEvent>((_, i) => ({
			id: this.memory.readUint32(ioEventPtr + i * 12),
			time: this.memory.readInt32(ioEventPtr + i * 12 + 4),
			duration: this.memory.readInt32(ioEventPtr + i * 12 + 8)
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

