import { Entity } from '../Entity';
import { Point } from '../Point';
import { RenderEngine } from '../RenderEngine';

const WIDTH = 100,
	HEIGHT = 25;

export class Tab extends Entity {
	constructor(private position: Point, private label: string) {
		super();
	}

	public render(renderEngine: RenderEngine, hovered: boolean, selected: boolean): void {
		if (hovered) {
			renderEngine.fillRect(this.position, WIDTH, HEIGHT, 'rgba(80, 144, 224, 0.5)');
		}

		renderEngine.rect(this.position, WIDTH - 1, HEIGHT - 1, 'black');

		renderEngine.text(this.position, this.label, { underline: selected });
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

