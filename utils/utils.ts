import { ProcessState, SchedulingStrategy } from './types';

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

export function prettyStrategy(strategy: SchedulingStrategy): string {
	switch (strategy) {
		case SchedulingStrategy.FIFO:
			return 'First In First Out';
		case SchedulingStrategy.SJF:
			return 'Shortest Job First';
		case SchedulingStrategy.SRT:
			return 'Shortest Remaining Time';
		case SchedulingStrategy.MLF:
			return 'Multi-Level Feedback';
		default:
			return 'whoops...';
	}
}

export function height(metrics: TextMetrics): number {
	return metrics.actualBoundingBoxAscent - metrics.actualBoundingBoxDescent;
}

