import { Entity } from './Entity';
import { Point } from './Point';
import { RenderEngine } from './RenderEngine';
import { Process } from './types';

interface CPUData {
	available: boolean;
	process: Process | null;
}

const WIDTH = 600,
	HEIGHT = 150,
	MARGIN = 25;

export class CPUIndicator extends Entity {
	constructor(private readonly core: number) {
		super();
	}

	public render(renderEngine: RenderEngine, data: CPUData): void {
		const center = new Point(-renderEngine.width / 2 + (WIDTH / 2 + 25), renderEngine.height / 2 - (HEIGHT / 2 + 25) - (HEIGHT + MARGIN) * this.core);

		renderEngine.fillRect(center.add(new Point(-WIDTH / 2 + 5, 0)), 10, HEIGHT, data.available ? 'green' : 'red');
		renderEngine.rect(center, WIDTH, HEIGHT, 'black');

		const label = `Core ${this.core}`;
		const metrics = renderEngine.measure(label);
		renderEngine.text(
			center.add(
				new Point(-WIDTH / 2 + 10 + metrics.width / 2 + 5, HEIGHT / 2 - (metrics.actualBoundingBoxAscent - metrics.actualBoundingBoxDescent) / 2 - 5)
			),
			label
		);

		if (data.available) {
			renderEngine.text(center.add(new Point(0, -14)), 'Free', { fontSize: 64 });
		} else {
			const progressBarWidth = WIDTH * 0.9;
			const yOffset = -HEIGHT / 2 + 40;
			const progressWidth = progressBarWidth * (data.process!.processorTime / data.process!.reqProcessorTime);

			renderEngine.fillRect(center.add(new Point(5 - progressBarWidth / 2 + progressWidth / 2, yOffset)), progressWidth, 25, 'green');
			renderEngine.rect(center.add(new Point(5, yOffset)), progressBarWidth, 25, 'black');
		}
	}

	public selectedBy(point: Point, getMetrics: (label: string) => TextMetrics): boolean {
		return false;
	}
}

