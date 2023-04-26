import { Entity, MouseData } from '../Entity';
import { View } from '../OSEngine';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { Process } from '../cpp-compat/Process';
import { prettyState } from '../utils';

export class ProcessStatsIndicator extends Entity {
	constructor(private position: Point, public readonly id: number) {
		super();
	}

	public render(renderEngine: RenderEngine, stats: Process, mousePos: Point | null): void {
		renderEngine.fillRect(this.position, 200, 150, 'white');
		renderEngine.rect(this.position, 199, 149, 'black');
		renderEngine.fillRect(this.position.add(new Point(0, 72.5)), 200, 15, 'gray');
		renderEngine.fillCircle(this.position.add(new Point(-5, 72.5)), 2, 'lightgray');
		renderEngine.fillCircle(this.position.add(new Point(0, 72.5)), 2, 'lightgray');
		renderEngine.fillCircle(this.position.add(new Point(5, 72.5)), 2, 'lightgray');
		renderEngine.line(this.position.add(new Point(88.5, 76.5)), this.position.add(new Point(96.5, 68.5)), 2, 'red');
		renderEngine.line(this.position.add(new Point(88.5, 68.5)), this.position.add(new Point(96.5, 76.5)), 2, 'red');

		renderEngine.text(this.position.add(new Point(0, 45)), `Process ${stats.pid} Stats`, { fontSize: 16 });

		(
			[
				['pid', 'PID'],
				['name', 'Name'],
				['arrivalTime', 'Arrival Time'],
				['doneTime', 'Completion Time'],
				['level', 'MLF Level'],
				['state', 'State']
			] as const
		).forEach(([stat, label], i) => {
			const text = `${label}: ${stat === 'state' ? prettyState(stats[stat]) : stats[stat]}`;
			const metrics = renderEngine.measure(text);

			renderEngine.text(this.position.add(new Point(-100 + metrics.width / 2 + 5, 75 - (7 + i * 14 + 40))), text);
		});

		if (mousePos && this._inDraggableArea(mousePos)) {
			renderEngine.setCursor('move');
		}
	}

	public selectedBy(point: Point, view: View): boolean {
		return (
			view === View.PROCESSES &&
			point.x >= this.position.x - 100 &&
			point.x <= this.position.x + 100 &&
			point.y >= this.position.y - 75 &&
			point.y <= this.position.y + 75
		);
	}

	public update(mouse: MouseData, selected: boolean): void {
		if (selected && mouse.position && mouse.down && this._inDraggableArea(mouse.position.subtract(mouse.delta))) {
			this.position = this.position.add(mouse.delta);
		}
	}

	public shouldClose(point: Point): boolean {
		return point.x >= this.position.x + 85 && point.x <= this.position.x + 100 && point.y >= this.position.y + 60 && point.y <= this.position.y + 75;
	}

	private _inDraggableArea(point: Point): boolean {
		return point.x >= this.position.x - 100 && point.x <= this.position.x + 85 && point.y >= this.position.y + 60 && point.y <= this.position.y + 75;
	}
}

