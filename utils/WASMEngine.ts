import { Memory } from './Memory';
import { Instruction, Ptr, RawMachineState, RawOSState } from './types';

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

	public getMachineState(): number {
		const ptr = this.module.asm.getMachineState();

		console.log('Num Cores', this.memory.getUint32(ptr));
		console.log('Clock Delay', this.memory.getUint32(ptr + 4));

		const availablePtr = this.memory.getUint32(ptr + 8);
		console.log(
			'Available',
			new Array(4).fill(null).map((_, i) => this.memory.getUint8(availablePtr + i) === 1)
		);
		return this.memory.getUint32(ptr + 8);
	}
}

