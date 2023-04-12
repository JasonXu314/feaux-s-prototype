import { Entity } from '../Entity';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { Process } from '../types';
import { height } from '../utils';

export class ReadyListIndicator extends Entity {
	private center: Point = new Point();
	private height: number = 0;
	private width: number = 0;

	public render(renderEngine: RenderEngine, readyList: Process[]): void {
		const { center, HEIGHT, WIDTH } = this._calculateDims(renderEngine, readyList.length);
		this.center = center;
		this.height = HEIGHT;
		this.width = WIDTH;

		renderEngine.text(center.add(new Point(0, HEIGHT / 2 + 20)), 'Ready List', { fontSize: 24 });
		renderEngine.rect(this.center, WIDTH, HEIGHT, 'black');

		if (readyList.length > 0) {
			readyList.forEach((process, i) => this._renderProcess(renderEngine, process, center.add(new Point(0, HEIGHT / 2 - (i * 100 + 50)))));
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
		renderEngine.rect(pos, 300, 100, 'black');

		const nameLabel = `Process: ${proc.name}`;
		const nameMetrics = renderEngine.measure(nameLabel);
		renderEngine.text(pos.add(new Point(-150 + nameMetrics.width / 2 + 5, 50 - height(nameMetrics) / 2 - 5)), nameLabel);

		const pidLabel = `PID: ${proc.id}`;
		const pidMetrics = renderEngine.measure(pidLabel);
		renderEngine.text(pos.add(new Point(-150 + pidMetrics.width / 2 + 5, 35 - height(nameMetrics) / 2 - 5)), pidLabel);

		const progressLabel = 'Progress:';
		const progressMetrics = renderEngine.measure(progressLabel);
		renderEngine.text(pos.add(new Point(-150 + pidMetrics.width / 2 + 35, -height(progressMetrics) / 2)), progressLabel);

		const progressBarWidth = 250;
		const yOffset = -20;
		const progressWidth = progressBarWidth * (proc.processorTime / proc.reqProcessorTime);

		renderEngine.fillRect(pos.add(new Point(5 - progressBarWidth / 2 + progressWidth / 2, yOffset)), progressWidth, 15, 'green');
		renderEngine.rect(pos.add(new Point(5, yOffset)), progressBarWidth, 15, 'black');
	}

	private _calculateDims(renderEngine: RenderEngine, count: number): { center: Point; HEIGHT: number; WIDTH: number } {
		const WIDTH = 300,
			HEIGHT = Math.max(count * 100, 100);

		return {
			HEIGHT,
			WIDTH,
			center: new Point(renderEngine.width / 2 - (WIDTH / 2 + 25), renderEngine.height / 2 - (HEIGHT / 2 + 50))
		};
	}
}

