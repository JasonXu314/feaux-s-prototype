import { Entity } from '../Entity';
import { View } from '../OSEngine';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { Process } from '../cpp-compat/Process';
import { ProcessState } from '../types';
import { STATUS_COLORS, height, prettyState } from '../utils';

const WIDTH = 180;

export class ProcessIndicator extends Entity {
	constructor(public position: Point, public readonly pid: number) {
		super();
	}

	public render(renderEngine: RenderEngine, proc: Process, selected: boolean): void {
		if (selected) {
			renderEngine.fillRect(this.position, WIDTH, 50, STATUS_COLORS.selected);
		}

		renderEngine.rect(this.position, WIDTH - 1, 49, 'black');

		const nameLabel = `Process: ${proc.name}`;
		const nameMetrics = renderEngine.measure(nameLabel);
		renderEngine.text(this.position.add(new Point(-90 + nameMetrics.width / 2 + 15, 25 - height(nameMetrics) / 2 - 5)), nameLabel);

		const pidLabel = `PID: ${proc.pid}`;
		const pidMetrics = renderEngine.measure(pidLabel);
		renderEngine.text(this.position.add(new Point(-90 + pidMetrics.width / 2 + 15, 10 - height(nameMetrics) / 2 - 5)), pidLabel);

		const statusLabel = `Status: ${prettyState(proc.state)}`;
		const statusMetrics = renderEngine.measure(statusLabel);
		renderEngine.text(this.position.add(new Point(-90 + statusMetrics.width / 2 + 15, -5 - height(nameMetrics) / 2 - 5)), statusLabel);

		switch (proc.state) {
			case ProcessState.DONE:
				renderEngine.fillRect(this.position.add(new Point(-85, 0)), 10, 50, STATUS_COLORS.green);
				break;
			case ProcessState.PROCESSING:
				renderEngine.fillRect(this.position.add(new Point(-85, 0)), 10, 50, STATUS_COLORS.yellow);
				break;
			case ProcessState.DEAD:
			case ProcessState.BLOCKED:
				renderEngine.fillRect(this.position.add(new Point(-85, 0)), 10, 50, STATUS_COLORS.red);
				break;
			case ProcessState.READY:
				renderEngine.fillRect(this.position.add(new Point(-85, 0)), 10, 50, STATUS_COLORS.blue);
				break;
			default:
				throw new Error(`Invalid process state: ${proc.state}`);
		}
	}

	public selectedBy(point: Point, view: View): boolean {
		return (
			view === View.PROCESSES &&
			point.x >= this.position.x - 100 &&
			point.x <= this.position.x + 100 &&
			point.y >= this.position.y - 25 &&
			point.y <= this.position.y + 25
		);
	}
}

