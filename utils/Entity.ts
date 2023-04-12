import { Point } from './Point';

export interface Metadata {
	selected: boolean;
	mouse: MouseData | null;
}

export type MouseData = { position: Point | null } & ({ down: true; delta: Point } | { down: false; delta: null });

export abstract class Entity {
	public abstract selectedBy(point: Point): boolean;
}

