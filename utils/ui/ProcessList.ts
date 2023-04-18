import { Entity, Metadata } from '../Entity';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { Process, ProcessState } from '../types';
import { STATUS_COLORS, height, prettyState } from '../utils';

const WIDTH = 180;

export class ProcessListIndicator extends Entity {
	private center: Point = new Point();
	private height: number = 0;
	private width: number = 0;

	public render(renderEngine: RenderEngine, processList: Process[], metadata: Metadata): void {
		const { center, HEIGHT, WIDTH } = this._calculateDims(renderEngine, processList.length);
		this.center = center;
		this.height = HEIGHT;
		this.width = WIDTH;

		renderEngine.text(center.add(new Point(0, HEIGHT / 2 + 20)), 'All Processes', { fontSize: 24 });
		renderEngine.rect(this.center, WIDTH - 1, HEIGHT - 1, 'black');

		if (processList.length > 0) {
			processList.forEach((process, i) => {
				const processPos = center.add(new Point(0, HEIGHT / 2 - (i * 50 + 25)));

				this._renderProcess(renderEngine, process, processPos, metadata.selected && this._processSelectedBy(metadata.mouse!.position!, processPos));
			});
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

	private _renderProcess(renderEngine: RenderEngine, proc: Process, pos: Point, selected: boolean): void {
		if (selected) {
			renderEngine.fillRect(pos, WIDTH, 50, 'rgba(80, 144, 224, 0.5)');
		}

		renderEngine.rect(pos, WIDTH - 1, 49, 'black');

		const nameLabel = `Process: ${proc.name}`;
		const nameMetrics = renderEngine.measure(nameLabel);
		renderEngine.text(pos.add(new Point(-90 + nameMetrics.width / 2 + 15, 25 - height(nameMetrics) / 2 - 5)), nameLabel);

		const pidLabel = `PID: ${proc.pid}`;
		const pidMetrics = renderEngine.measure(pidLabel);
		renderEngine.text(pos.add(new Point(-90 + pidMetrics.width / 2 + 15, 10 - height(nameMetrics) / 2 - 5)), pidLabel);

		const statusLabel = `Status: ${prettyState(proc.state)}`;
		const statusMetrics = renderEngine.measure(statusLabel);
		renderEngine.text(pos.add(new Point(-90 + statusMetrics.width / 2 + 15, -5 - height(nameMetrics) / 2 - 5)), statusLabel);

		switch (proc.state) {
			case ProcessState.DONE:
				renderEngine.fillRect(pos.add(new Point(-85, 0)), 10, 50, STATUS_COLORS.green);
				break;
			case ProcessState.PROCESSING:
				renderEngine.fillRect(pos.add(new Point(-85, 0)), 10, 50, STATUS_COLORS.yellow);
				break;
			case ProcessState.BLOCKED:
				renderEngine.fillRect(pos.add(new Point(-85, 0)), 10, 50, STATUS_COLORS.red);
				break;
			case ProcessState.READY:
				renderEngine.fillRect(pos.add(new Point(-85, 0)), 10, 50, STATUS_COLORS.blue);
				break;
			default:
				throw new Error(`Invalid process state: ${proc.state}`);
		}
	}

	private _processSelectedBy(point: Point, processPos: Point): boolean {
		return point.x >= processPos.x - 100 && point.x <= processPos.x + 100 && point.y >= processPos.y - 25 && point.y <= processPos.y + 25;
	}

	private _calculateDims(renderEngine: RenderEngine, count: number): { center: Point; HEIGHT: number; WIDTH: number } {
		const HEIGHT = Math.max(count * 50, 50);

		return {
			HEIGHT,
			WIDTH,
			center: new Point(-renderEngine.width / 2 + 340 + (WIDTH / 2 + 25), renderEngine.height / 2 - (HEIGHT / 2 + 50))
		};
	}
}

