import { View } from './OSEngine';
import { ProcessState, Regs, SchedulingStrategy } from './types';

// colors pulled from astro design (https://www.astrouxds.com/patterns/status-system/)
export const STATUS_COLORS = {
	green: '#56F000',
	yellow: '#FCE83A',
	red: '#FF3838',
	blue: '#2DCCFF',
	selected: 'rgba(80, 144, 224, 0.5)'
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
		case SchedulingStrategy.RT_FIFO:
			return 'Real-Time First In First Out';
		case SchedulingStrategy.RT_EDF:
			return 'Real-Time Earliest Deadline First';
		case SchedulingStrategy.RT_LST:
			return 'Real-Time Least Slack Time';
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

export function getRegister(reg: string): Regs {
	switch (reg) {
		case 'rax':
			return Regs.RAX;
		case 'rcx':
			return Regs.RCX;
		case 'rdx':
			return Regs.RDX;
		case 'rbx':
			return Regs.RBX;
		case 'rsi':
			return Regs.RSI;
		case 'rdi':
			return Regs.RDI;
		case 'rsp':
			return Regs.RSP;
		case 'rbp':
			return Regs.RBP;
		case 'r8':
			return Regs.R8;
		case 'r9':
			return Regs.R9;
		case 'r10':
			return Regs.R10;
		case 'r11':
			return Regs.R11;
		case 'r12':
			return Regs.R12;
		case 'r13':
			return Regs.R13;
		case 'r14':
			return Regs.R14;
		case 'r15':
			return Regs.R15;
		default:
			throw new Error(`Unrecognized register mnemonic ${reg}`);
	}
}

export function height(metrics: TextMetrics): number {
	return metrics.actualBoundingBoxAscent - metrics.actualBoundingBoxDescent;
}

export function formatHex(number: number, digits: number): string {
	let str = number.toString(16);
	while (str.length < digits) str = '0' + str;

	return '0x' + str;
}

