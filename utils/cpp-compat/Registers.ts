import { Memory } from '../Memory';

export class Registers {
	public static readonly SIZE = 8;

	private _rip: number = -1;
	private _rdi: number = -1;

	public static readFrom(memory: Memory, ptr: number): Registers;
	public static readFrom(memory: Memory, ptr: number, count: number): Registers[];
	public static readFrom(memory: Memory, ptr: number, count?: number): Registers | Registers[] {
		if (count === undefined) {
			const regs = new Registers();

			regs._rip = memory.readUint32(ptr);
			regs._rdi = memory.readInt32(ptr + 4);

			return regs;
		} else {
			return new Array(count).fill(null).map((_, i) => this.readFrom(memory, ptr + i * this.SIZE));
		}
	}

	public get rip(): number {
		return this._rip;
	}

	public get rdi(): number {
		return this._rdi;
	}
}

