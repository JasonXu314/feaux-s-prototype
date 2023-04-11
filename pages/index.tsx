import { NextPage } from 'next/types';
import { Button } from 'primereact/button';
import { useEffect, useState } from 'react';
import { WASMEngine } from '../utils/WASMEngine';
import { Opcode } from '../utils/types';

const Index: NextPage = () => {
	const [wasmEngine, setWASMEngine] = useState<WASMEngine | null>(null);

	useEffect(() => {
		const wasmEngine = new WASMEngine((window as any).Module);

		setWASMEngine(wasmEngine);

		// const int = setInterval(() => {
		// 	console.log(wasmEngine.getMachineState());
		// }, 1000);

		// return () => {
		// 	clearInterval(int);
		// };
	}, []);

	return (
		<div>
			<h1>Test</h1>
			<Button label="Button" icon="pi pi-check" onClick={() => wasmEngine?.addProcess([{ opcode: Opcode.WORK, operand: 10 }], 'test process')} />
		</div>
	);
};

export default Index;

