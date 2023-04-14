import type { Entity, MouseData } from './Entity';
import { Point } from './Point';
import { RenderEngine } from './RenderEngine';
import { WASMEngine } from './WASMEngine';
import { Instruction, Opcode } from './types';
import { CPUIndicator } from './ui/CPU';
import { ProcessListIndicator } from './ui/ProcessList';
import { ReadyListIndicator } from './ui/ReadyList';

export interface ProgramDescriptor {
	name: string;
	file: string;
}

const DEFAULT_PROGRAMS: ProgramDescriptor[] = [
	{ name: 'Standard Worker', file: 'worker' },
	{ name: 'IO Worker', file: 'io' },
	{ name: 'More Work + IO', file: 'more-work' },
	{ name: 'More IO + Work', file: 'more-io' }
];

interface EngineEvents {
	entityClicked: (entity: Entity, metadata: { button: MouseButton; spacePos: Point; pagePos: Point }) => void;
	entityDblClicked: (entity: Entity) => void;
	click: (evt: MouseEvent) => void;
	finishLoad: () => void;
}

export enum MouseButton {
	LEFT,
	MIDDLE,
	RIGHT,
	BACK,
	FORWARD
}

export class OSEngine {
	private readonly context: CanvasRenderingContext2D;
	private readonly entities: Entity[] = [];
	private readonly renderEngine: RenderEngine;
	private readonly programs: Map<string, Instruction[]> = new Map();

	private readonly cpuIndicators: CPUIndicator[] = [];
	private readonly readyListIndicator: ReadyListIndicator = new ReadyListIndicator();
	private readonly processListIndicator: ProcessListIndicator = new ProcessListIndicator();

	private _nextTick: number = -1;
	private _selectedEntity: Entity | null = null;
	private _mousePos: Point | null = null;
	private _mouseDown = false;
	private _mouseDelta: Point | null = null;
	private _numIndicators: number = 0;

	private _listeners: { [K in keyof EngineEvents]: EngineEvents[K][] };

	private mouseListener: (evt: MouseEvent) => void = (evt) => {
		if (this._mousePos) {
			this._mousePos = this.renderEngine.canvasToSpace(new Point(evt.offsetX, evt.offsetY));

			if (this._mouseDelta) {
				this._mouseDelta.x += evt.movementX;
				this._mouseDelta.y -= evt.movementY;
			}
		}
	};

	constructor(private readonly canvas: HTMLCanvasElement, private readonly wasmEngine: WASMEngine) {
		const ctx = canvas.getContext('2d');

		if (ctx) {
			this.context = ctx;
			this.renderEngine = new RenderEngine(ctx, canvas);

			for (; this._numIndicators < 4; this._numIndicators++) {
				const indicator = new CPUIndicator(this._numIndicators);

				this.cpuIndicators.push(indicator);
				this.entities.push(indicator);
			}
			this.entities.push(this.readyListIndicator);
			this.entities.push(this.processListIndicator);

			this._listeners = { entityClicked: [], click: [], entityDblClicked: [], finishLoad: [] };

			Promise.all(DEFAULT_PROGRAMS.map((descriptor) => this._preloadProgram(descriptor))).then(() => this._listeners.finishLoad.forEach((cb) => cb()));

			canvas.addEventListener('mouseout', () => {
				this._mousePos = null;

				canvas.removeEventListener('mousemove', this.mouseListener);
			});

			canvas.addEventListener('mouseover', (evt) => {
				this._mousePos = new Point(evt.offsetX, evt.offsetY);

				canvas.addEventListener('mousemove', this.mouseListener);
			});

			canvas.addEventListener('mousedown', () => {
				this._mouseDown = true;
				this._mouseDelta = new Point();
			});

			canvas.addEventListener('mouseup', (evt: MouseEvent) => {
				this._mouseDown = false;
				this._mouseDelta = null;

				if (this._selectedEntity) {
					for (const listener of this._listeners.entityClicked) {
						listener(this._selectedEntity, {
							button: evt.button,
							spacePos: this._mousePos!,
							pagePos: this.renderEngine.spaceToCanvas(this._mousePos!).add(new Point(0, 36))
						});
					}
				} else {
					for (const listener of this._listeners.click) {
						listener(evt);
					}
				}
			});

			canvas.addEventListener('dblclick', () => {
				if (this._selectedEntity) {
					for (const listener of this._listeners.entityDblClicked) {
						listener(this._selectedEntity);
					}
				}
			});

			canvas.addEventListener('contextmenu', (evt: MouseEvent) => {
				if (this._selectedEntity) {
					evt.preventDefault();
				}
			});
		} else {
			throw new Error('Unable to get canvas context');
		}
	}

	public start(): void {
		this._tick();
	}

	public on<T extends keyof EngineEvents>(evt: T, listener: EngineEvents[T]): () => void {
		this._listeners[evt].push(listener);

		return () => {
			this._listeners[evt].splice(this._listeners[evt].indexOf(listener), 1);
		};
	}

	public stop(): void {
		cancelAnimationFrame(this._nextTick);
	}

	public getPrograms(): string[] {
		return [...this.programs.keys()];
	}

	public spawn(program: string): void {
		this.wasmEngine.addProcess(this.programs.get(program)!, program);
	}

	private _tick(): void {
		this._nextTick = requestAnimationFrame(() => this._tick());

		if (!this._mouseDown) {
			this._updateSelectedEntity();
		}

		if (this._selectedEntity) {
			this.canvas.style.cursor = 'pointer';
		} else {
			this.canvas.style.cursor = 'unset';
		}

		this.context.clearRect(0, 0, this.canvas.width, this.canvas.height);
		this.context.fillStyle = 'white';
		this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);
		this.context.fillStyle = 'black';

		const machineState = this.wasmEngine.getMachineState();
		const osState = this.wasmEngine.getOSState();
		this.cpuIndicators.forEach((cpu, i) =>
			cpu.render(this.renderEngine, { available: machineState.available[i], process: machineState.available[i] ? null : machineState.runningProcess[i] })
		);

		this.readyListIndicator.render(this.renderEngine, osState.readyList);
		this.processListIndicator.render(this.renderEngine, osState.processList, {
			selected: this._selectedEntity === this.processListIndicator,
			mouse: { delta: this._mouseDelta!, down: this._mouseDown, position: this._mousePos } as MouseData
		});

		if (this._mouseDelta) {
			this._mouseDelta = new Point();
		}
	}

	public compileProgram(name: string, code: string): void {
		const instructionList = code.split('\n').map((line) => {
			const [mnemonic, operand] = line.split(' ');

			return this._compile(mnemonic, operand);
		});

		this.programs.set(name, instructionList);
	}

	private _preloadProgram({ name, file }: ProgramDescriptor): Promise<void> {
		return fetch(`/default-programs/${file}.fsp`)
			.then((res) => res.text())
			.then((code) => this.compileProgram(name, code))
			.catch((err) => console.error(`Error loading default program ${name}`, err));
	}

	private _compile(mnemonic: string, operand: string): Instruction {
		switch (mnemonic) {
			case 'nop':
				return { opcode: Opcode.NOP, operand: -1 };
			case 'work':
				return { opcode: Opcode.WORK, operand: parseInt(operand) };
			case 'io':
				return { opcode: Opcode.IO, operand: parseInt(operand) };
			default:
				throw new Error(`Unrecognized mnemonic ${mnemonic}`);
		}
	}

	private _updateSelectedEntity(): void {
		if (this._mousePos) {
			const reversedEntities = this.entities.reduce<Entity[]>((arr, entity) => [entity, ...arr], []);

			for (const entity of reversedEntities) {
				if (entity.selectedBy(this._mousePos)) {
					this._selectedEntity = entity;
					return;
				}
			}
		}

		this._selectedEntity = null;
	}
}

