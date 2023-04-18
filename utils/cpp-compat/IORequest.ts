import { Memory } from '../Memory';

export class IORequest {
	public static readonly SIZE = 8;

	private _pid: number = -1;
	private _duration: number = -1;

	public static readFrom(memory: Memory, ptr: number): IORequest;
	public static readFrom(memory: Memory, ptr: number, count: number): IORequest[];
	public static readFrom(memory: Memory, ptr: number, count?: number): IORequest | IORequest[] {
		if (count === undefined) {
			const regs = new IORequest();

			regs._pid = memory.readUint32(ptr);
			regs._duration = memory.readInt32(ptr + 4);

			return regs;
		} else {
			return new Array(count).fill(null).map((_, i) => this.readFrom(memory, ptr + i * this.SIZE));
		}
	}

	public get pid(): number {
		return this._pid;
	}

	public get duration(): number {
		return this._duration;
	}
}

