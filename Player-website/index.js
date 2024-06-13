
window.onSpotifyWebPlaybackSDKReady = () => {
	const token = 'BQCw57hydoNqW3AloWqjgqpsItFxhZUCYkLcm5S9XjQll2xbxP6RHsPLF1-C5VGP7_h7rrlAsjeJCW02VPBWLXTZn1IbyqYpZ96ODjREfPKUFiQPAL8RDCGGn0AFm6TVwyV6uCVS-w1Le_8k15vXnAELSH61BR5kNZF2l-C9RL7BRzWcqE5PxRM_uqfC108ZyFRP8CaruXG37GtugMK0LII2sxA22qdn';
	const player = new Spotify.Player({
			name: 'Hidden Web Player',
			getOAuthToken: cb => { cb(token); },
			volume: 0.1
	});

	player.addListener('initialization_error', ({ message }) => { console.error(message); });
	player.addListener('authentication_error', ({ message }) => { console.error(message); });
	player.addListener('account_error', ({ message }) => { console.error(message); });
	player.addListener('playback_error', ({ message }) => { console.error(message); });
	player.addListener('not_ready', ({ device_id }) => { console.log('Device ID has gone offline', device_id); });
	player.addListener('player_state_changed', state => { console.log(state); });
	player.addListener('ready', ({ device_id }) => {
		console.log('Ready with Device ID', device_id);
		// Transfer playback to this device
		fetch(`https://api.spotify.com/v1/me/player`, {
				method: 'PUT',
				body: JSON.stringify({
						device_ids: [device_id],
						play: true,
				}),
				headers: {
						'Content-Type': 'application/json',
						'Authorization': `Bearer ${token}`
				},
		});
});

	document.getElementById('togglePlay').onclick = function() {
		player.togglePlay();
	};

	player.connect();
}