const urlParams = new URLSearchParams(window.location.search);
let code = urlParams.get('code'), error = urlParams.get('error');

//redirect to spotify login
if (!code && !error) {
	const generateRandomString = (length) => {
		const possible = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
		const values = crypto.getRandomValues(new Uint8Array(length));
		return Array.from(values)
			.map((x) => possible[x % possible.length])
			.join('');
	}
	
	const sha256 = async (plain) => {
		const encoder = new TextEncoder();
		const data = encoder.encode(plain);
		return window.crypto.subtle.digest('SHA-256', data);
	}
	
	const base64encode = (input) => {
		return btoa(String.fromCharCode(...new Uint8Array(input)))
			.replace(/=/g, '')
			.replace(/\+/g, '-')
			.replace(/\//g, '_');
	}
	
	const generateCodeVerifierAndChallenge = async () => {
		const codeVerifier = generateRandomString(64);
		const hashed = await sha256(codeVerifier);
		const codeChallenge = base64encode(hashed);
		return { codeVerifier, codeChallenge };
	};
	
	const clientId = '244ba241897d4c969d1260ad0c844f91';
	const scopeValue = 'user-read-private user-read-email';
	const authUrl = new URL('https://accounts.spotify.com/authorize');
	const redirectUri = 'http://fede-ai.github.io/Lyrics-viewer/redirect.html';
	
	generateCodeVerifierAndChallenge().then(({ codeVerifier, codeChallenge }) => {
		const params = {
			response_type: 'code',
			client_id: clientId,
			scope: scopeValue,
			code_challenge_method: 'S256',
			code_challenge : codeChallenge,
			redirect_uri: redirectUri,
		};
	
		window.localStorage.setItem('code_verifier', codeVerifier);
	
		authUrl.search = new URLSearchParams(params).toString();
		window.location.href = authUrl.toString();
	});
}
else if (code) {
	window.localStorage.setItem('code', code);
	window.location.href = 'http://fede-ai.github.io/Lyrics-viewer/success.html';
}
else {
	window.localStorage.setItem('error', error);
	window.location.href = 'http://fede-ai.github.io/Lyrics-viewer/fail.html';
}
