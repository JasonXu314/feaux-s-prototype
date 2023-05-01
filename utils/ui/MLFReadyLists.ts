import { Entity } from '../Entity';
import { View } from '../OSEngine';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { Process } from '../cpp-compat/Process';
import { height } from '../utils';

export class MLFReadyListsIndicator extends Entity {
	private center: Point = new Point();
	private height: number = 0;
	private width: number = 0;

	public render(renderEngine: RenderEngine, readyLists: Process[][]): void {
		const { center, HEIGHT, WIDTH } = this._calculateDims(
			renderEngine,
			readyLists.reduce((max, list) => (list.length > max ? list.length : max), 0)
		);
		this.center = center;
		this.height = HEIGHT;
		this.width = WIDTH;

		renderEngine.text(center.add(new Point(0, HEIGHT / 2 + 35)), 'MLF Lists', { fontSize: 24 });
		renderEngine.rect(this.center, WIDTH - 1, HEIGHT - 1, 'black');

		readyLists.forEach((readyList, i) => {
			const xOFfset = -WIDTH / 2 + i * 200 + 100;

			renderEngine.text(center.add(new Point(xOFfset, HEIGHT / 2 + 15)), `Priority ${6 - i}`, { fontSize: 18 });
			if (readyList.length > 0) {
				readyList.forEach((process, j) => this._renderProcess(renderEngine, process, center.add(new Point(xOFfset, HEIGHT / 2 - (j * 100 + 50)))));
			} else {
				renderEngine.text(center.add(new Point(xOFfset, -5)), 'Empty', { fontSize: 32 });
			}
		});
	}

	public selectedBy(point: Point, view: View): boolean {
		return (
			view === View.PROCESSES &&
			point.x >= this.center.x - this.width / 2 &&
			point.x <= this.center.x + this.width / 2 &&
			point.y >= this.center.y - this.height / 2 &&
			point.y <= this.center.y + this.height / 2
		);
	}

	public _renderProcess(renderEngine: RenderEngine, proc: Process, pos: Point): void {
		renderEngine.rect(pos, 199, 99, 'black');

		const nameLabel = `Process: ${proc.name}`;
		const nameMetrics = renderEngine.measure(nameLabel);
		renderEngine.text(pos.add(new Point(-100 + nameMetrics.width / 2 + 5, 50 - height(nameMetrics) / 2 - 5)), nameLabel);

		const pidLabel = `PID: ${proc.pid}`;
		const pidMetrics = renderEngine.measure(pidLabel);
		renderEngine.text(pos.add(new Point(-100 + pidMetrics.width / 2 + 5, 35 - height(nameMetrics) / 2 - 5)), pidLabel);

		const progressLabel = 'Progress:';
		const progressMetrics = renderEngine.measure(progressLabel);
		renderEngine.text(pos.add(new Point(-100 + pidMetrics.width / 2 + 35, -height(progressMetrics) / 2)), progressLabel);

		const progressBarWidth = 160;
		const yOffset = -20;
		const progressWidth = progressBarWidth * (proc.processorTime / proc.reqProcessorTime);

		renderEngine.fillRect(pos.add(new Point(5 - progressBarWidth / 2 + progressWidth / 2, yOffset)), progressWidth, 15, 'green');
		renderEngine.rect(pos.add(new Point(5, yOffset)), progressBarWidth, 15, 'black');
	}

	private _calculateDims(renderEngine: RenderEngine, maxCount: number): { center: Point; HEIGHT: number; WIDTH: number } {
		const WIDTH = 1200,
			HEIGHT = Math.max(maxCount * 100, 100);

		return {
			HEIGHT,
			WIDTH,
			center: new Point(-renderEngine.width / 2 + (WIDTH / 2 + 250), renderEngine.height / 2 - (HEIGHT / 2 + 95))
		};
	}
}

