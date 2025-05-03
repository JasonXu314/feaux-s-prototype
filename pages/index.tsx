import { NextPage } from 'next/types';
import { Button } from 'primereact/button';
import { Checkbox } from 'primereact/checkbox';
import { ContextMenu } from 'primereact/contextmenu';
import { Dialog } from 'primereact/dialog';
import { Dropdown } from 'primereact/dropdown';
import { InputNumber } from 'primereact/inputnumber';
import { InputText } from 'primereact/inputtext';
import { InputTextarea } from 'primereact/inputtextarea';
import { MenuItem } from 'primereact/menuitem';
import { ToggleButton } from 'primereact/togglebutton';
import { MouseEvent, useCallback, useEffect, useMemo, useRef, useState } from 'react';
import Canvas from '../components/Canvas';
import { MouseButton, OSEngine } from '../utils/OSEngine';
import { Point } from '../utils/Point';
import { WASMEngine } from '../utils/WASMEngine';
import { SchedulingStrategy } from '../utils/types';
import { CPUIndicator } from '../utils/ui/CPU';
import { ProcessIndicator } from '../utils/ui/Process';
import { prettyStrategy } from '../utils/utils';

const Index: NextPage = () => {
	const [wasmEngine, setWASMEngine] = useState<WASMEngine | null>(null);
	const [osEngine, setOSEngine] = useState<OSEngine | null>(null);
	const [availablePrograms, setAvilablePrograms] = useState<string[]>([]);
	const [selectedProgram, setSelectedProgram] = useState<string>('Standard Worker');
	const [deadline, setDeadline] = useState<number>(-1);
	const [period, setPeriod] = useState<number>(-1);
	const [delay, setDelay] = useState<number>(0);
	const [periodic, setPeriodic] = useState<boolean>(false);
	const [paused, setPaused] = useState<boolean>(false);
	const [clockDelay, setClockDelay] = useState<number>(500);
	const [writingProgram, setWritingProgram] = useState<boolean>(false);
	const [programName, setProgramName] = useState<string>('');
	const [programCode, setProgramCode] = useState<string>('');
	const [schedulingStrategy, setSchedulingStrategy] = useState<SchedulingStrategy>(SchedulingStrategy.FIFO);
	const [numCores, setNumCores] = useState<number>(2);
	const [numIODevices, setNumIODevices] = useState<number>(1);
	const ctxMenu = useRef<ContextMenu | null>(null);
	const ctxEvt = useRef<MouseEvent<HTMLCanvasElement, globalThis.MouseEvent> | null>(null);
	const [menuItems, setMenuItems] = useState<MenuItem[]>([]);

	const rt = useMemo(
		() => [SchedulingStrategy.RT_FIFO, SchedulingStrategy.RT_EDF, SchedulingStrategy.RT_LST].includes(schedulingStrategy),
		[schedulingStrategy]
	);

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

			if (!rt) {
				setDeadline(-1);
				setPeriodic(false);
			}
		}
	}, [osEngine, schedulingStrategy, rt]);

	const onLoad = useCallback((wasmEngine: WASMEngine, osEngine: OSEngine) => {
		setWASMEngine(wasmEngine);
		setOSEngine(osEngine);

		(window as any).wasmEngine = wasmEngine;

		osEngine.on('entityClicked', (entity, { button, spacePos }) => {
			if (button === MouseButton.RIGHT) {
				if (entity instanceof CPUIndicator) {
					setMenuItems([
						{
							label: 'View Registers',
							icon: 'pi pi-fw pi-search',
							command: () => {
								osEngine.showCPURegisters(entity.core, spacePos.add(new Point(50, -200)));
							}
						}
					]);

					setTimeout(() => {
						if (ctxMenu.current) {
							ctxMenu.current.show(ctxEvt.current!);
						}
					}, 0);
				} else if (entity instanceof ProcessIndicator) {
					setMenuItems([
						{
							label: 'View Registers',
							icon: 'pi pi-fw pi-search',
							command: () => {
								osEngine.showProcessRegisters(entity.pid, spacePos.add(new Point(50, -200)));
							}
						},
						{
							label: 'View Stats',
							icon: 'pi pi-fw pi-list',
							command: () => {
								osEngine.showProcessStats(entity.pid, spacePos.add(new Point(50, -150)));
							}
						}
					]);

					setTimeout(() => {
						if (ctxMenu.current) {
							ctxMenu.current.show(ctxEvt.current!);
						}
					}, 0);
				}
			}
		});
	}, []);

	return (
		<div>
			<div className="flex flex-column gap-1">
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
						<div className="flex align-items-center gap-2">
							<label className="font-bold">Machine&nbsp;Specs</label>
							<InputNumber
								suffix=" cores"
								showButtons
								inputStyle={{ maxWidth: '10em' }}
								value={numCores}
								onValueChange={(evt) => {
									if (evt.value) {
										setNumCores(evt.value);
										osEngine?.setNumCores(evt.value);
									}
								}}
							/>
							<InputNumber
								suffix=" I/O Devices"
								showButtons
								inputStyle={{ maxWidth: '10em' }}
								value={numIODevices}
								onValueChange={(evt) => {
									if (evt.value) {
										setNumIODevices(evt.value);
										osEngine?.setNumIODevices(evt.value);
									}
								}}
							/>
						</div>
						<div className="flex align-items-center gap-2">
							<label htmlFor="clock-delay" className="font-bold">
								Clock&nbsp;Delay
							</label>
							<InputNumber
								inputId="clock-delay"
								suffix="ms"
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
				</div>
				<div className="flex flex-row px-4 py-1 gap-4">
					<div className="flex align-items-center gap-2">
						<div className="p-inputgroup">
							<Button
								label="Run"
								onClick={() => {
									if (periodic) {
										osEngine?.dispatch(selectedProgram, period, deadline, delay);
									} else {
										osEngine?.spawn(selectedProgram, rt ? deadline : -1);
									}
								}}
							/>
							<Dropdown value={selectedProgram} onChange={(evt) => setSelectedProgram(evt.value)} options={availablePrograms} />
						</div>
						{rt && (
							<>
								<label htmlFor="deadline" className="font-bold">
									Deadline
								</label>
								<InputNumber
									inputId="deadline"
									suffix=" ticks"
									value={deadline}
									onValueChange={(evt) => {
										if (evt.value) {
											setDeadline(evt.value);
										}
									}}
								/>
								<label htmlFor="periodic" className="font-bold">
									Periodic
								</label>
								<Checkbox inputId="periodic" checked={periodic} onChange={(evt) => setPeriodic(!!evt.checked)} />
								{periodic && (
									<>
										<label htmlFor="period" className="font-bold">
											Period
										</label>
										<InputNumber
											inputId="period"
											suffix=" ticks"
											value={period}
											onValueChange={(evt) => {
												if (evt.value) {
													setPeriod(evt.value);
												}
											}}
										/>
										<label htmlFor="delay" className="font-bold">
											Delay
										</label>
										<InputNumber
											inputId="delay"
											suffix=" ticks"
											value={delay}
											onValueChange={(evt) => {
												if (evt.value) {
													setDelay(evt.value);
												}
											}}
										/>
									</>
								)}
							</>
						)}
					</div>
					<Dropdown
						value={schedulingStrategy}
						onChange={(evt) => setSchedulingStrategy(evt.value)}
						options={[
							SchedulingStrategy.FIFO,
							SchedulingStrategy.SJF,
							SchedulingStrategy.SRT,
							SchedulingStrategy.MLF,
							SchedulingStrategy.RT_FIFO,
							SchedulingStrategy.RT_EDF,
							SchedulingStrategy.RT_LST
						].map((strategy) => ({
							value: strategy,
							label: prettyStrategy(strategy)
						}))}
					/>
					<Button label="Create&nbsp;Program" icon="pi pi-plus" className="flex-none" onClick={() => setWritingProgram(!writingProgram)} />
				</div>
			</div>
			<Canvas onLoad={onLoad} evtRef={ctxEvt} />
			<ContextMenu model={menuItems} ref={ctxMenu} />
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

