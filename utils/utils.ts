import { ProcessState } from './types';

export function prettyState(state: ProcessState): string {
	switch (state) {
		case ProcessState.BLOCKED:
			return 'BLOCKED';
		case ProcessState.DONE:
			return 'DONE';
		case ProcessState.PROCESSING:
			return 'PROCESSING';
		case ProcessState.READY:
			return 'READY';
		default:
			return 'whoops...';
	}
}

export function height(metrics: TextMetrics): number {
	return metrics.actualBoundingBoxAscent - metrics.actualBoundingBoxDescent;
}
