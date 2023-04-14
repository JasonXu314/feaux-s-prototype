import { Memory } from '../Memory';

export class IOEvent {
	public static readonly SIZE = 12;

	public _id: number = -1;
	public _time: number = -1;
	public _duration: number = -1;

	public static readFrom(memory: Memory, ptr: number): IOEvent;
	public static readFrom(memory: Memory, ptr: number, count: number): IOEvent[];
	public static readFrom(memory: Memory, ptr: number, count?: number): IOEvent | IOEvent[] {
		if (count === undefined) {
			const proc = new IOEvent();

			proc._id = memory.readUint32(ptr);
			proc._time = memory.readInt32(ptr + 4);
			proc._duration = memory.readInt32(ptr + 8);

			return proc;
		} else {
			return new Array(count).fill(null).map((_, i) => this.readFrom(memory, ptr + i * this.SIZE));
		}
	}

	public get id(): number {
		return this._id;
	}

	public get time(): number {
		return this._time;
	}

	public get duration(): number {
		return this._duration;
	}
}

