import { View } from './OSEngine';
import { ProcessState, SchedulingStrategy } from './types';

// colors pulled from astro design (https://www.astrouxds.com/patterns/status-system/)
export const STATUS_COLORS = {
	green: '#56F000',
	yellow: '#FCE83A',
	red: '#FF3838',
	blue: '#2DCCFF'
} as const;

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

export function prettyView(view: View): string {
	switch (view) {
		case View.HARDWARE:
			return 'Hardware View';
		case View.PROCESSES:
			return 'Process View';
		default:
			return 'whoops...';
	}
}

export function height(metrics: TextMetrics): number {
	return metrics.actualBoundingBoxAscent - metrics.actualBoundingBoxDescent;
}

