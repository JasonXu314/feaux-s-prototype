import { Memory } from '../Memory';
import { ProcessState } from '../types';
import { Registers } from './Registers';

export class Process {
	public static readonly SIZE = 40 + Registers.SIZE;

	private _pid: number = -1;
	private _name: string = '';
	private _arrivalTime: number = -1;
	private _deadline: number = -1;
	private _doneTime: number = -1;
	private _reqProcessorTime: number = -1;
	private _processorTime: number = -1;
	private _level: number = -1;
	private _processorTimeOnLevel: number = -1;
	private _state: ProcessState = ProcessState.READY;
	private _registers: Registers = Registers.DEF_REGSTATE;

	private constructor() {}

	public static readFrom(memory: Memory, ptr: number): Process;
	public static readFrom(memory: Memory, ptr: number, count: number): Process[];
	public static readFrom(memory: Memory, ptr: number, count?: number): Process | Process[] {
		if (count === undefined) {
			const proc = new Process();

			proc._pid = memory.readUint32(ptr);
			proc._name = memory.readString(memory.readUint32(ptr + 4));
			proc._arrivalTime = memory.readInt32(ptr + 8);
			proc._deadline = memory.readInt32(ptr + 12);
			proc._doneTime = memory.readInt32(ptr + 16);
			proc._reqProcessorTime = memory.readInt32(ptr + 20);
			proc._processorTime = memory.readInt32(ptr + 24);
			proc._level = memory.readInt32(ptr + 28);
			proc._processorTimeOnLevel = memory.readUint32(ptr + 32);
			proc._state = memory.readUint32(ptr + 36);
			proc._registers = Registers.readFrom(memory, ptr + 40);

			return proc;
		} else {
			return new Array(count).fill(null).map((_, i) => this.readFrom(memory, ptr + this.SIZE * i));
		}
	}

	public get pid(): number {
		return this._pid;
	}

	public get name(): string {
		return this._name;
	}

	public get arrivalTime(): number {
		return this._arrivalTime;
	}

	public get doneTime(): number {
		return this._doneTime;
	}

	public get reqProcessorTime(): number {
		return this._reqProcessorTime;
	}

	public get processorTime(): number {
		return this._processorTime;
	}

	public get deadline(): number {
		return this._deadline;
	}

	public get level(): number {
		return this._level;
	}

	public get processorTimeOnLevel(): number {
		return this._processorTimeOnLevel;
	}

	public get state(): ProcessState {
		return this._state;
	}

	public get registers(): Registers {
		return this._registers;
	}
}

