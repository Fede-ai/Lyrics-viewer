if (typeof(getToken) !== typeof(Function)) {
	window.location.href = 'http://fede-ai.github.io/Lyrics-viewer/external.html';
}

window.onSpotifyWebPlaybackSDKReady = () => {
	let token = getToken();
	while (!token) {
		await (new Promise(r => setTimeout(r, 1000)));
		token = getToken();
	}

	const player = new Spotify.Player({
		name: 'Lyrics-viewer player',
		getOAuthToken: cb => { cb(token); },
		volume: 0.2
	});

	player.addListener('ready', ({ device_id }) => {
		console.log('Ready with Device ID', device_id);
	});

	player.addListener('not_ready', ({ device_id }) => {
		console.log('Device ID has gone offline', device_id);
	});

	player.addListener('initialization_error', ({ message }) => {
		console.error(message);
	});

	player.addListener('authentication_error', ({ message }) => {
		console.error(message);
	});

	player.addListener('account_error', ({ message }) => {
		console.error(message);
	});

	player.connect();
}