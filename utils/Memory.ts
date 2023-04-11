export class Memory {
	private readonly dv: DataView;

	constructor(memory: WebAssembly.Memory) {
		this.dv = new DataView(memory.buffer);
	}

	public getUint8(offset: number): number {
		return this.dv.getUint8(offset);
	}

	public getUint32(offset: number): number {
		return this.dv.getUint32(offset, true);
	}

	public getInt32(offset: number): number {
		return this.dv.getInt32(offset, true);
	}
}

