import { Entity } from '../Entity';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { Process, ProcessState } from '../types';
import { height, prettyState } from '../utils';

export class ProcessListIndicator extends Entity {
	private center: Point = new Point();
	private height: number = 0;
	private width: number = 0;

	public render(renderEngine: RenderEngine, processList: Process[]): void {
		const { center, HEIGHT, WIDTH } = this._calculateDims(renderEngine, processList.length);
		this.center = center;
		this.height = HEIGHT;
		this.width = WIDTH;

		renderEngine.text(center.add(new Point(0, HEIGHT / 2 + 20)), 'Ready List', { fontSize: 24 });
		renderEngine.rect(this.center, WIDTH, HEIGHT, 'black');

		if (processList.length > 0) {
			processList.forEach((process, i) => this._renderProcess(renderEngine, process, center.add(new Point(0, HEIGHT / 2 - (i * 50 + 25)))));
		} else {
			renderEngine.text(center.add(new Point(0, -5)), 'Empty', { fontSize: 32 });
		}
	}

	public selectedBy(point: Point): boolean {
		return (
			point.x >= this.center.x - this.width / 2 &&
			point.x <= this.center.x + this.width / 2 &&
			point.y >= this.center.y - this.height / 2 &&
			point.y <= this.center.y + this.height / 2
		);
	}

	public _renderProcess(renderEngine: RenderEngine, proc: Process, pos: Point): void {
		renderEngine.rect(pos, 200, 50, 'black');

		const nameLabel = `Process: ${proc.name}`;
		const nameMetrics = renderEngine.measure(nameLabel);
		renderEngine.text(pos.add(new Point(-100 + nameMetrics.width / 2 + 15, 25 - height(nameMetrics) / 2 - 5)), nameLabel);

		const pidLabel = `PID: ${proc.id}`;
		const pidMetrics = renderEngine.measure(pidLabel);
		renderEngine.text(pos.add(new Point(-100 + pidMetrics.width / 2 + 15, 10 - height(nameMetrics) / 2 - 5)), pidLabel);

		const statusLabel = `Status: ${prettyState(proc.state)}`;
		const statusMetrics = renderEngine.measure(statusLabel);
		renderEngine.text(pos.add(new Point(-100 + statusMetrics.width / 2 + 15, -5 - height(nameMetrics) / 2 - 5)), statusLabel);

		switch (proc.state) {
			case ProcessState.DONE:
				renderEngine.fillRect(pos.add(new Point(-95, 0)), 10, 50, 'green');
				break;
			case ProcessState.PROCESSING:
				renderEngine.fillRect(pos.add(new Point(-95, 0)), 10, 50, 'yellow');
				break;
			case ProcessState.BLOCKED:
				renderEngine.fillRect(pos.add(new Point(-95, 0)), 10, 50, 'red');
				break;
			case ProcessState.READY:
				renderEngine.fillRect(pos.add(new Point(-95, 0)), 10, 50, 'blue');
				break;
			default:
				throw new Error('Invalid process state');
		}
	}

	private _calculateDims(renderEngine: RenderEngine, count: number): { center: Point; HEIGHT: number; WIDTH: number } {
		const WIDTH = 200,
			HEIGHT = Math.max(count * 50, 50);

		return {
			HEIGHT,
			WIDTH,
			center: new Point(renderEngine.width / 2 - (WIDTH / 2 + 25) - 325, renderEngine.height / 2 - (HEIGHT / 2 + 50))
		};
	}
}
