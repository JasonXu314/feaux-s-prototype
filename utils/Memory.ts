export class Memory {
	private readonly dv: DataView;

	constructor(memory: WebAssembly.Memory) {
		this.dv = new DataView(memory.buffer);
	}

	public readUint8(offset: number): number {
		return this.dv.getUint8(offset);
	}

	public writeUint8(offset: number, value: number): void {
		this.dv.setUint8(offset, value);
	}

	public readUint32(offset: number): number {
		return this.dv.getUint32(offset, true);
	}

	public readInt32(offset: number): number {
		return this.dv.getInt32(offset, true);
	}
}

