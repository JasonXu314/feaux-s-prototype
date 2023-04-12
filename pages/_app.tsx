import { AppProps } from 'next/app';
import 'primeicons/primeicons.css';
import 'primereact/resources/primereact.min.css';
import 'primereact/resources/themes/lara-light-blue/theme.css';

const App: React.FC<AppProps> = ({ Component, pageProps }) => {
	return <Component {...pageProps} />;
};

export default App;

