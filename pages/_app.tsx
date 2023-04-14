import { AppProps } from 'next/app';
import Head from 'next/head';
import 'primeicons/primeicons.css';
import PrimeReact from 'primereact/api';
import 'primereact/resources/primereact.min.css';
import 'primereact/resources/themes/lara-light-blue/theme.css';

PrimeReact.ripple = true;

const App: React.FC<AppProps> = ({ Component, pageProps }) => {
	return (
		<>
			<Head>
				<title>Feaux-S</title>
			</Head>
			<Component {...pageProps} />
		</>
	);
};

export default App;

