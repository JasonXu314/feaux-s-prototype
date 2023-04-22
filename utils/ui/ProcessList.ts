import { Entity, Metadata } from '../Entity';
import { View } from '../OSEngine';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { Process } from '../cpp-compat/Process';
import { ProcessIndicator } from './Process';

const WIDTH = 180;

export class ProcessListIndicator extends Entity {
	private readonly processIndicators: ProcessIndicator[] = [];

	private center: Point = new Point();
	private height: number = 0;
	private width: number = 0;

	public render(renderEngine: RenderEngine, processList: Process[], metadata: Metadata): void {
		const { center, HEIGHT, WIDTH } = this._calculateDims(renderEngine, processList.length);
		this.center = center;
		this.height = HEIGHT;
		this.width = WIDTH;

		renderEngine.text(center.add(new Point(0, HEIGHT / 2 + 15)), 'All Processes', { fontSize: 24 });
		renderEngine.rect(this.center, WIDTH - 1, HEIGHT - 1, 'black');

		if (processList.length > 0) {
			while (this.processIndicators.length < processList.length)
				this.processIndicators.push(
					new ProcessIndicator(
						center.add(new Point(0, HEIGHT / 2 - (this.processIndicators.length * 50 + 25))),
						processList[this.processIndicators.length].pid
					)
				);

			processList.forEach((process, i) =>
				this.processIndicators[i].render(
					renderEngine,
					process,
					metadata.mouse?.position ? this.processIndicators[i].selectedBy(metadata.mouse.position, View.PROCESSES) : false
				)
			);
		} else {
			renderEngine.text(center.add(new Point(0, -5)), 'Empty', { fontSize: 32 });
		}
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

	public selectedProcess(point: Point, view: View): ProcessIndicator | undefined {
		return this.processIndicators.find((indicator) => indicator.selectedBy(point, view));
	}

	public get length(): number {
		return this.processIndicators.length;
	}

	private _calculateDims(renderEngine: RenderEngine, count: number): { center: Point; HEIGHT: number; WIDTH: number } {
		const HEIGHT = Math.max(count * 50, 50);

		return {
			HEIGHT,
			WIDTH,
			center: new Point(-renderEngine.width / 2 + 25 + (WIDTH / 2 + 25), renderEngine.height / 2 - (HEIGHT / 2 + 75))
		};
	}
}

