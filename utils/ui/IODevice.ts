import { Entity } from '../Entity';
import { View } from '../OSEngine';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { DeviceState } from '../types';
import { STATUS_COLORS } from '../utils';

const WIDTH = 315,
	HEIGHT = 150,
	MARGIN = 25;

export class IODeviceIndicator extends Entity {
	private center: Point = new Point();

	constructor(private readonly id: number) {
		super();
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

	public render(renderEngine: RenderEngine, state: DeviceState): void {
		this.center = this._calculateCenter(renderEngine);

		renderEngine.rect(this.center, WIDTH - 1, HEIGHT - 1, 'black');
		renderEngine.fillRect(this.center.add(new Point(-WIDTH / 2 + 5, 0)), 10, HEIGHT, state.pid === 0 ? STATUS_COLORS.green : STATUS_COLORS.blue);

		const label = `IO Device ${this.id}`;
		const metrics = renderEngine.measure(label);
		renderEngine.text(
			this.center.add(
				new Point(-WIDTH / 2 + 10 + metrics.width / 2 + 5, HEIGHT / 2 - (metrics.actualBoundingBoxAscent - metrics.actualBoundingBoxDescent) / 2 - 5)
			),
			label
		);

		if (state.pid === 0) {
			renderEngine.text(this.center.add(new Point(0, -14)), 'Free', { fontSize: 64 });
		} else {
			const progressBarWidth = WIDTH * 0.9;
			const yOffset = -HEIGHT / 2 + 40;
			const progressWidth = progressBarWidth * (state.progress / state.duration);

			renderEngine.fillRect(this.center.add(new Point(5 - progressBarWidth / 2 + progressWidth / 2, yOffset)), progressWidth, 25, 'green');
			renderEngine.rect(this.center.add(new Point(5, yOffset)), progressBarWidth, 25, 'black');

			const processNameLabel = `Process PID: ${state.pid}`;
			const nameMetrics = renderEngine.measure(processNameLabel);
			renderEngine.text(this.center.add(new Point(-WIDTH / 2 + 25 + nameMetrics.width / 2, 40)), processNameLabel);
		}
	}

	private _calculateCenter(renderEngine: RenderEngine): Point {
		return new Point(-renderEngine.width / 2 + 350 + (WIDTH / 2 + 50), renderEngine.height / 2 - (HEIGHT / 2 + 50) - (HEIGHT + MARGIN) * this.id);
	}
}

