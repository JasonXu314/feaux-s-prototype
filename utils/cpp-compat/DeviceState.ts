import { Memory } from '../Memory';

export class DeviceState {
	public static readonly SIZE = 12;

	private _pid: number = -1;
	private _duration: number = -1;
	private _progress: number = -1;

	public static readFrom(memory: Memory, ptr: number): DeviceState;
	public static readFrom(memory: Memory, ptr: number, count: number): DeviceState[];
	public static readFrom(memory: Memory, ptr: number, count?: number): DeviceState | DeviceState[] {
		if (count === undefined) {
			const state = new DeviceState();

			state._pid = memory.readUint8(ptr);
			state._duration = memory.readInt32(ptr + 4);
			state._progress = memory.readInt32(ptr + 8);

			return state;
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

	public get progress(): number {
		return this._progress;
	}
}

