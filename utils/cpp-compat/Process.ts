import { Memory } from '../Memory';
import { ProcessState } from '../types';
import { IOEvent } from './IOEvent';

export class Process {
	public static readonly SIZE = 36;

	private _id: number = -1;
	private _name: string = '';
	private _arrivalTime: number = -1;
	private _doneTime: number = -1;
	private _reqProcessorTime: number = -1;
	private _processorTime: number = -1;
	private _state: ProcessState = ProcessState.READY;
	private _numIOEvents: number = -1;
	private _ioEvents: IOEvent[] = [];

	private constructor() {}

	public static readFrom(memory: Memory, ptr: number): Process;
	public static readFrom(memory: Memory, ptr: number, count: number): Process[];
	public static readFrom(memory: Memory, ptr: number, count?: number): Process | Process[] {
		if (count === undefined) {
			const proc = new Process();

			proc._id = memory.readUint32(ptr);
			proc._name = memory.readString(memory.readUint32(ptr + 4));
			proc._arrivalTime = memory.readInt32(ptr + 8);
			proc._doneTime = memory.readInt32(ptr + 12);
			proc._reqProcessorTime = memory.readInt32(ptr + 16);
			proc._processorTime = memory.readInt32(ptr + 20);
			proc._state = memory.readUint32(ptr + 24);
			proc._numIOEvents = memory.readUint32(ptr + 28);
			proc._ioEvents = IOEvent.readFrom(memory, memory.readUint32(ptr + 32), proc.numIOEvents);

			return proc;
		} else {
			return new Array(count).fill(null).map((_, i) => this.readFrom(memory, ptr + this.SIZE * i));
		}
	}

	public get id(): number {
		return this._id;
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

	public get state(): ProcessState {
		return this._state;
	}

	public get numIOEvents(): number {
		return this._numIOEvents;
	}

	public get ioEvents(): IOEvent[] {
		return this._ioEvents;
	}
}

