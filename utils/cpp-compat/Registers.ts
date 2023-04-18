import { Memory } from '../Memory';

export class Registers {
	public static readonly SIZE = 68;

	private _rip: number = -1;
	private _rax: number = -1;
	private _rcx: number = -1;
	private _rdx: number = -1;
	private _rbx: number = -1;
	private _rsi: number = -1;
	private _rdi: number = -1;
	private _rsp: number = -1;
	private _rbp: number = -1;
	private _r8: number = -1;
	private _r9: number = -1;
	private _r10: number = -1;
	private _r11: number = -1;
	private _r12: number = -1;
	private _r13: number = -1;
	private _r14: number = -1;
	private _r15: number = -1;

	public static readFrom(memory: Memory, ptr: number): Registers;
	public static readFrom(memory: Memory, ptr: number, count: number): Registers[];
	public static readFrom(memory: Memory, ptr: number, count?: number): Registers | Registers[] {
		if (count === undefined) {
			const regs = new Registers();

			regs._rip = memory.readUint32(ptr);
			regs._rax = memory.readUint32(ptr + 4);
			regs._rcx = memory.readUint32(ptr + 8);
			regs._rdx = memory.readUint32(ptr + 12);
			regs._rbx = memory.readUint32(ptr + 16);
			regs._rsi = memory.readUint32(ptr + 20);
			regs._rdi = memory.readUint32(ptr + 24);
			regs._rsp = memory.readUint32(ptr + 28);
			regs._rbp = memory.readUint32(ptr + 32);
			regs._r8 = memory.readUint32(ptr + 36);
			regs._r9 = memory.readUint32(ptr + 40);
			regs._r10 = memory.readUint32(ptr + 44);
			regs._r11 = memory.readUint32(ptr + 48);
			regs._r12 = memory.readUint32(ptr + 52);
			regs._r13 = memory.readUint32(ptr + 56);
			regs._r14 = memory.readUint32(ptr + 60);
			regs._r15 = memory.readUint32(ptr + 64);

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

	public get rax(): number {
		return this._rax;
	}

	public get rcx(): number {
		return this._rcx;
	}

	public get rdx(): number {
		return this._rdx;
	}

	public get rbx(): number {
		return this._rbx;
	}

	public get rsi(): number {
		return this._rsi;
	}

	public get rsp(): number {
		return this._rsp;
	}

	public get rbp(): number {
		return this._rbp;
	}

	public get r8(): number {
		return this._r8;
	}

	public get r9(): number {
		return this._r9;
	}

	public get r10(): number {
		return this._r10;
	}

	public get r11(): number {
		return this._r11;
	}

	public get r12(): number {
		return this._r12;
	}

	public get r13(): number {
		return this._r13;
	}

	public get r14(): number {
		return this._r14;
	}

	public get r15(): number {
		return this._r15;
	}
}

