import { NextPage } from 'next/types';
import { Button } from 'primereact/button';
import { Dialog } from 'primereact/dialog';
import { Dropdown } from 'primereact/dropdown';
import { InputNumber } from 'primereact/inputnumber';
import { InputText } from 'primereact/inputtext';
import { InputTextarea } from 'primereact/inputtextarea';
import { ToggleButton } from 'primereact/togglebutton';
import { useCallback, useEffect, useState } from 'react';
import Canvas from '../components/Canvas';
import { OSEngine } from '../utils/OSEngine';
import { WASMEngine } from '../utils/WASMEngine';
import { SchedulingStrategy } from '../utils/types';
import { prettyStrategy } from '../utils/utils';

const Index: NextPage = () => {
	const [wasmEngine, setWASMEngine] = useState<WASMEngine | null>(null);
	const [osEngine, setOSEngine] = useState<OSEngine | null>(null);
	const [availablePrograms, setAvilablePrograms] = useState<string[]>([]);
	const [selectedProgram, setSelectedProgram] = useState<string>('Standard Worker');
	const [paused, setPaused] = useState<boolean>(false);
	const [clockDelay, setClockDelay] = useState<number>(500);
	const [writingProgram, setWritingProgram] = useState<boolean>(false);
	const [programName, setProgramName] = useState<string>('');
	const [programCode, setProgramCode] = useState<string>('');
	const [schedulingStrategy, setSchedulingStrategy] = useState<SchedulingStrategy>(SchedulingStrategy.FIFO);

	useEffect(() => {
		if (osEngine) {
			const cancel = osEngine.on('finishLoad', () => {
				setAvilablePrograms(osEngine.getPrograms());
			});

			return () => {
				cancel();
				osEngine.stop();
			};
		}
	}, [osEngine]);

	useEffect(() => {
		if (osEngine) {
			osEngine.setSchedulingStrategy(schedulingStrategy);
		}
	}, [osEngine, schedulingStrategy]);

	const onLoad = useCallback((wasmEngine: WASMEngine, osEngine: OSEngine) => {
		setWASMEngine(wasmEngine);
		setOSEngine(osEngine);
	}, []);

	return (
		<div>
			<div className="flex gap-4 px-4 py-1">
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
					<Button label="Run" onClick={() => osEngine?.spawn(selectedProgram)} />
					<Dropdown value={selectedProgram} onChange={(evt) => setSelectedProgram(evt.value)} options={availablePrograms} />
				</div>
				<Dropdown
					value={schedulingStrategy}
					onChange={(evt) => setSchedulingStrategy(evt.value)}
					options={[SchedulingStrategy.FIFO, SchedulingStrategy.SJF, SchedulingStrategy.SRT, SchedulingStrategy.MLF].map((strategy) => ({
						value: strategy,
						label: prettyStrategy(strategy)
					}))}
				/>
				<Button label="Create&nbsp;Program" icon="pi pi-plus" className="flex-none" onClick={() => setWritingProgram(!writingProgram)} />
				<div className="flex align-items-center gap-2">
					<label htmlFor="clock-delay" className="font-bold">
						Clock&nbsp;Delay
					</label>
					<InputNumber
						inputId="clock-delay"
						value={clockDelay}
						onValueChange={(evt) => {
							if (evt.value) {
								setClockDelay(evt.value);
								wasmEngine?.setClockDelay(evt.value);
							}
						}}
					/>
				</div>
			</div>
			<Canvas onLoad={onLoad} />
			<Dialog
				header="Program Editor"
				visible={writingProgram}
				onHide={() => {
					setWritingProgram(false);
					setProgramName('');
					setProgramCode('');
				}}>
				<span className="p-float-label mt-4">
					<InputText id="program-name" value={programName} onChange={(evt) => setProgramName(evt.target.value)} />
					<label htmlFor="program-name">Program Name</label>
				</span>
				<span className="p-float-label mt-5">
					<InputTextarea id="program-code" value={programCode} onChange={(evt) => setProgramCode(evt.target.value)} rows={20} cols={60} />
					<label htmlFor="program-code">Program Code</label>
				</span>
				<Button
					label="Add Program"
					onClick={() => {
						osEngine?.compileProgram(programName, programCode);
						setAvilablePrograms(osEngine!.getPrograms());
						setWritingProgram(false);
						setProgramName('');
						setProgramCode('');
					}}
				/>
			</Dialog>
		</div>
	);
};

export default Index;

