import { NextPage } from 'next/types';
import { Button } from 'primereact/button';
import { useCallback, useRef } from 'react';
import { OSEngine } from '../utils/OSEngine';
import { WASMEngine } from '../utils/WASMEngine';
import { Opcode } from '../utils/types';

const Index: NextPage = () => {
	const wasmEngine = useRef<WASMEngine | null>(null);
	const osEngine = useRef<OSEngine | null>(null);

	const setup = useCallback((canvas: HTMLCanvasElement) => {
		const we = new WASMEngine((window as any).Module);
		const oe = new OSEngine(canvas, we);

		wasmEngine.current = we;
		osEngine.current = oe;

		oe.start();
	}, []);

	return (
		<div>
			<Button
				label="Button"
				icon="pi pi-check"
				onClick={() => {
					wasmEngine.current?.addProcess([{ opcode: Opcode.WORK, operand: 10 }], 'test process');
				}}
			/>
			<canvas height="800" width="1200" ref={(elem) => elem && setup(elem)} />
		</div>
	);
};

export default Index;

