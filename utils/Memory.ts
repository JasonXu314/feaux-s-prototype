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

	public writeUint32(offset: number, value: number): void {
		this.dv.setUint32(offset, value, true);
	}

	public readInt32(offset: number): number {
		return this.dv.getInt32(offset, true);
	}

	public writeInt32(offset: number, value: number): void {
		this.dv.setInt32(offset, value, true);
	}

	public readString(ptr: number): string {
		let str = '',
			i = 0,
			codePoint = -1;

		while ((codePoint = this.readUint8(ptr + i)) !== 0) {
			str += String.fromCodePoint(codePoint);
			i++;
		}

		return str;
	}

	public writeString(ptr: number, str: string): void {
		for (let i = 0; i < str.length; i++) {
			this.writeUint8(ptr + i, str.codePointAt(i)!);
		}
	}
}

