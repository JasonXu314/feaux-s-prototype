import { Entity } from '../Entity';
import { View } from '../OSEngine';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { Process } from '../types';
import { STATUS_COLORS, prettyState } from '../utils';

interface CPUData {
	available: boolean;
	process: Process | null;
}

const WIDTH = 315,
	HEIGHT = 150,
	MARGIN = 25;

export class CPUIndicator extends Entity {
	private center: Point = new Point();

	constructor(private readonly core: number) {
		super();
	}

	public render(renderEngine: RenderEngine, data: CPUData): void {
		this.center = this._calculateCenter(renderEngine);

		renderEngine.rect(this.center, WIDTH - 1, HEIGHT - 1, 'black');
		renderEngine.fillRect(this.center.add(new Point(-WIDTH / 2 + 5, 0)), 10, HEIGHT, data.available ? STATUS_COLORS.green : STATUS_COLORS.blue);

		const label = `Core ${this.core}`;
		const metrics = renderEngine.measure(label);
		renderEngine.text(
			this.center.add(
				new Point(-WIDTH / 2 + 10 + metrics.width / 2 + 5, HEIGHT / 2 - (metrics.actualBoundingBoxAscent - metrics.actualBoundingBoxDescent) / 2 - 5)
			),
			label
		);

		if (data.available) {
			renderEngine.text(this.center.add(new Point(0, -14)), 'Free', { fontSize: 64 });
		} else {
			const progressBarWidth = WIDTH * 0.9;
			const yOffset = -HEIGHT / 2 + 40;
			const progressWidth = progressBarWidth * (data.process!.processorTime / data.process!.reqProcessorTime);

			renderEngine.fillRect(this.center.add(new Point(5 - progressBarWidth / 2 + progressWidth / 2, yOffset)), progressWidth, 25, 'green');
			renderEngine.rect(this.center.add(new Point(5, yOffset)), progressBarWidth, 25, 'black');

			const processNameLabel = `Process: ${data.process!.name}`;
			const nameMetrics = renderEngine.measure(processNameLabel);
			renderEngine.text(this.center.add(new Point(-WIDTH / 2 + 25 + nameMetrics.width / 2, 40)), processNameLabel);

			const pidLabel = `PID: ${data.process!.pid}`;
			const pidMetrics = renderEngine.measure(pidLabel);
			renderEngine.text(this.center.add(new Point(-WIDTH / 2 + 25 + pidMetrics.width / 2, 25)), pidLabel);

			const stateLabel = `State: ${prettyState(data.process!.state)}`;
			const stateMetrics = renderEngine.measure(stateLabel);
			renderEngine.text(this.center.add(new Point(-WIDTH / 2 + 25 + stateMetrics.width / 2, 10)), stateLabel);
		}
	}

	public selectedBy(point: Point, view: View): boolean {
		return (
			view === View.HARDWARE &&
			point.x >= this.center.x - WIDTH / 2 &&
			point.x <= this.center.x + WIDTH / 2 &&
			point.y <= this.center.y + HEIGHT / 2 &&
			point.y >= this.center.y - HEIGHT / 2
		);
	}

	private _calculateCenter(renderEngine: RenderEngine): Point {
		return new Point(-renderEngine.width / 2 + (WIDTH / 2 + 50), renderEngine.height / 2 - (HEIGHT / 2 + 50) - (HEIGHT + MARGIN) * this.core);
	}
}

