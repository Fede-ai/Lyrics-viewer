if (typeof(getToken) !== typeof(Function)) {
	window.location.href = 'http://fede-ai.github.io/Lyrics-viewer/external.html';
}

window.onSpotifyWebPlaybackSDKReady = () => {
	const token = 'BQAtPcjUhO3Fj1Bd3JDVScyI9WZfyQBgSNa-KuktMNIjdbh_v8QH1CLgLsKEplmFn6AEvz_nnFrHdDzCg8lWzowdXXEZVlbYZHrveoBMFX6w8TRAzw_1TzcWVsgQiooVaDD063GH2uf7pY2igOTtDiEPY8flw2wknzQd5ROPJxi19p2FTcckzzuUzM4SUBGFn79fupMTyv52w6hOWao-lNyA6-tXPZmgqIRfLdvX';
	const player = new Spotify.Player({
			name: 'Web Playback SDK Quick Start Player',
			getOAuthToken: cb => { cb(token); },
			volume: 0.5
	});

	// Ready
	player.addListener('ready', ({ device_id }) => {
			console.log('Ready with Device ID', device_id);
	});

	// Not Ready
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

	document.getElementById('togglePlay').onclick = function() {
		player.togglePlay();
	};

	player.connect();
}