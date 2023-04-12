import { NextPage } from 'next/types';
import { Button } from 'primereact/button';
import { Dropdown } from 'primereact/dropdown';
import { InputNumber } from 'primereact/inputnumber';
import { ToggleButton } from 'primereact/togglebutton';
import { useCallback, useEffect, useState } from 'react';
import Canvas from '../components/Canvas';
import { OSEngine } from '../utils/OSEngine';
import { WASMEngine } from '../utils/WASMEngine';

const Index: NextPage = () => {
	const [wasmEngine, setWASMEngine] = useState<WASMEngine | null>(null);
	const [osEngine, setOSEngine] = useState<OSEngine | null>(null);
	const [availablePrograms, setAvilablePrograms] = useState<string[]>([]);
	const [selectedProgram, setSelectedProgram] = useState<string>('Standard Worker');
	const [paused, setPaused] = useState<boolean>(false);
	const [clockDelay, setClockDelay] = useState<number>(500);

	useEffect(() => {
		if (osEngine) {
			const cancel = osEngine.on('finishLoad', () => {
				setAvilablePrograms(osEngine.getPrograms());
			});

			return () => {
				cancel();
			};
		}
	}, [osEngine]);

	const onLoad = useCallback((wasmEngine: WASMEngine, osEngine: OSEngine) => {
		setWASMEngine(wasmEngine);
		setOSEngine(osEngine);
	}, []);

	return (
		<div>
			<div className="flex gap-4">
				<ToggleButton
					checked={paused}
					onChange={(evt) => {
						setPaused(evt.value);

						if (evt.value) {
							wasmEngine?.pause();
						} else {
							wasmEngine?.unpause();
						}
					}}
					onLabel="Paused"
					offLabel="Running"
					className="flex-none"
					style={{
						backgroundColor: paused ? 'var(--red-500)' : 'var(--green-600)',
						border: paused ? 'var(--red-600)' : 'var(--green-700)',
						color: 'white'
					}}
				/>
				<div className="p-inputgroup">
					<Button label="Spawn" onClick={() => osEngine?.spawn(selectedProgram)} />
					<Dropdown value={selectedProgram} onChange={(evt) => setSelectedProgram(evt.value)} options={availablePrograms} />
				</div>
				<InputNumber
					value={clockDelay}
					onValueChange={(evt) => {
						if (evt.value) {
							setClockDelay(evt.value);
							wasmEngine?.setClockDelay(evt.value);
						}
					}}
				/>
			</div>
			<Canvas onLoad={onLoad} />
		</div>
	);
};

export default Index;

