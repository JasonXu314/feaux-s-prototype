import { AppProps } from 'next/app';
import Head from 'next/head';
import 'primeicons/primeicons.css';
import 'primereact/resources/primereact.min.css';
import 'primereact/resources/themes/lara-light-blue/theme.css';

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

