import { NextPage } from 'next/types';
import { Button } from 'primereact/button';
import { useEffect, useState } from 'react';
import { WASMEngine } from '../utils/WASMEngine';

const Index: NextPage = () => {
	const [wasmEngine, setWASMEngine] = useState<WASMEngine | null>(null);

	useEffect(() => {
		const wasmEngine = new WASMEngine((window as any).Module);

		setWASMEngine(wasmEngine);

		console.log(wasmEngine.getMachineState());
	}, []);

	return (
		<div>
			<h1>Test</h1>
			<Button label="Button" icon="pi pi-check" />
		</div>
	);
};

export default Index;

