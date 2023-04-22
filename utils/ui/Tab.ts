import { Entity } from '../Entity';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';
import { STATUS_COLORS } from '../utils';

const WIDTH = 100,
	HEIGHT = 25;

export class Tab extends Entity {
	constructor(private position: Point, private label: string) {
		super();
	}

	public render(renderEngine: RenderEngine, hovered: boolean, selected: boolean): void {
		if (hovered) {
			renderEngine.fillRect(this.position, WIDTH, HEIGHT, STATUS_COLORS.selected);
		}

		renderEngine.rect(this.position, WIDTH - 1, HEIGHT - 1, 'black');
		renderEngine.text(this.position, this.label, { underline: selected });

		if (selected) {
			if (hovered) {
				renderEngine.fillRect(this.position.add(new Point(0, -HEIGHT / 2 + 0.5)), WIDTH, 2, STATUS_COLORS.selected);
			} else {
				renderEngine.fillRect(this.position.add(new Point(0, -HEIGHT / 2 + 0.5)), WIDTH - 2, 2, 'white');
			}
		}
	}

	public selectedBy(point: Point): boolean {
		return (
			point.x >= this.position.x - WIDTH / 2 &&
			point.x <= this.position.x + WIDTH / 2 &&
			point.y >= this.position.y - HEIGHT / 2 &&
			point.y <= this.position.y + HEIGHT / 2
		);
	}
}

