import { Memory } from '../Memory';
import { DEF_REGSTATE, Registers } from '../types';
import { Registers as Regs } from './Registers';

export class CPUState {
	public static readonly SIZE = 4 + Regs.SIZE;

	private _available: boolean = false;
	private _regstate: Registers = DEF_REGSTATE;

	public static readFrom(memory: Memory, ptr: number): CPUState;
	public static readFrom(memory: Memory, ptr: number, count: number): CPUState[];
	public static readFrom(memory: Memory, ptr: number, count?: number): CPUState | CPUState[] {
		if (count === undefined) {
			const state = new CPUState();

			state._available = memory.readUint8(ptr) === 1;
			state._regstate = Regs.readFrom(memory, ptr + 4);

			return state;
		} else {
			return new Array(count).fill(null).map((_, i) => this.readFrom(memory, ptr + i * this.SIZE));
		}
	}

	public get available(): boolean {
		return this._available;
	}

	public get regstate(): Registers {
		return this._regstate;
	}
}

