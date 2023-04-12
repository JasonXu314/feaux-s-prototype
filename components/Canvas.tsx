import { memo, useCallback, useRef } from 'react';
import { OSEngine } from '../utils/OSEngine';
import { WASMEngine } from '../utils/WASMEngine';

interface Props {
	onLoad(wasmEngine: WASMEngine, osEngine: OSEngine): void;
}

const Canvas: React.FC<Props> = ({ onLoad }) => {
	const wasmEngine = useRef<WASMEngine | null>(null);
	const osEngine = useRef<OSEngine | null>(null);

	const setup = useCallback(
		(canvas: HTMLCanvasElement) => {
			const we = new WASMEngine((window as any).Module);
			const oe = new OSEngine(canvas, we);

			wasmEngine.current = we;
			osEngine.current = oe;

			onLoad(we, oe);

			oe.start();
		},
		[onLoad]
	);

	return <canvas height="800" width="1800" ref={(elem) => elem && setup(elem)} />;
};

export default memo(Canvas, ({ onLoad: prevOnLoad }, { onLoad }) => prevOnLoad === onLoad);

