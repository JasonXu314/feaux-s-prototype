import { Entity, MouseData } from '../Entity';
import { View } from '../OSEngine';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { CPUState } from '../cpp-compat/CPUState';
import { Process } from '../cpp-compat/Process';
import { ProcessState } from '../types';
import { formatHex } from '../utils';

export class RegistersIndicator extends Entity {
	constructor(private position: Point, public readonly id: number, public readonly type: 'process' | 'cpu') {
		super();
	}

	public render(renderEngine: RenderEngine, cpuOrProcess: CPUState | Process, mousePos: Point | null): void {
		const regs = cpuOrProcess instanceof CPUState ? cpuOrProcess.regstate : cpuOrProcess.registers;

		renderEngine.fillRect(this.position, 200, 400, 'white');
		renderEngine.rect(this.position, 199, 399, 'black');
		renderEngine.fillRect(this.position.add(new Point(0, 192.5)), 200, 15, 'gray');
		renderEngine.fillCircle(this.position.add(new Point(-5, 192.5)), 2, 'lightgray');
		renderEngine.fillCircle(this.position.add(new Point(0, 192.5)), 2, 'lightgray');
		renderEngine.fillCircle(this.position.add(new Point(5, 192.5)), 2, 'lightgray');
		renderEngine.line(this.position.add(new Point(88.5, 196.5)), this.position.add(new Point(96.5, 188.5)), 2, 'red');
		renderEngine.line(this.position.add(new Point(88.5, 188.5)), this.position.add(new Point(96.5, 196.5)), 2, 'red');

		renderEngine.text(
			this.position.add(new Point(0, 170)),
			cpuOrProcess instanceof CPUState ? `Core ${this.id} Registers` : `Process ${this.id} Registers`,
			{ fontSize: 16 }
		);

		const procRunning = cpuOrProcess instanceof Process ? cpuOrProcess.state !== ProcessState.DONE : false;

		(
			['flags', 'rip', '', '', 'rax', 'rcx', 'rdx', 'rbx', 'rsi', 'rdi', 'rsp', 'rbp', 'r8', 'r9', 'r10', 'r11', 'r12', 'r13', 'r14', 'r15'] as const
		).forEach((reg, i) => {
			if (reg !== '') {
				const text = `%${reg}: ${procRunning ? '????????' : formatHex(regs[reg], 8)}`;
				const metrics = renderEngine.measure(text);

				renderEngine.text(this.position.add(new Point(-100 + metrics.width / 2 + 5, 200 - (7 + i * 14 + 40))), text);
			}
		});

		if (mousePos && this._inDraggableArea(mousePos)) {
			renderEngine.setCursor('move');
		}
	}

	public selectedBy(point: Point, view: View): boolean {
		return (
			(this.type === 'cpu' ? view === View.HARDWARE : view === View.PROCESSES) &&
			point.x >= this.position.x - 100 &&
			point.x <= this.position.x + 100 &&
			point.y >= this.position.y - 200 &&
			point.y <= this.position.y + 200
		);
	}

	public update(mouse: MouseData, selected: boolean): void {
		if (selected && mouse.position && mouse.down && this._inDraggableArea(mouse.position.subtract(mouse.delta))) {
			this.position = this.position.add(mouse.delta);
		}
	}

	public shouldClose(point: Point): boolean {
		return point.x >= this.position.x + 85 && point.x <= this.position.x + 100 && point.y >= this.position.y + 185 && point.y <= this.position.y + 200;
	}

	private _inDraggableArea(point: Point): boolean {
		return point.x >= this.position.x - 100 && point.x <= this.position.x + 85 && point.y >= this.position.y + 185 && point.y <= this.position.y + 200;
	}
}

