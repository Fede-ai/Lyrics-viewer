const urlParams = new URLSearchParams(window.location.search);
let code = urlParams.get('code'), error = urlParams.get('error');

const redirectUri = 'http://fede-ai.github.io/Lyrics-viewer/redirect.html';
const clientId = '244ba241897d4c969d1260ad0c844f91';

//not from my app
if (typeof(sendToCpp) !== typeof(Function)) {
	window.location.href = 'http://fede-ai.github.io/Lyrics-viewer/external.html';
}
//redirect to spotify login
else if (!code && !error) {
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
	
	const scopeValue = 'user-read-private user-read-email';
	const authUrl = new URL('https://accounts.spotify.com/authorize');
	
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
//success after spotify login
else if (code) {
	const getToken = async (c) => {
		const payload = {
			method: 'POST',
			headers: {
				'Content-Type': 'application/x-www-form-urlencoded',
			},
			body: new URLSearchParams({
				client_id: clientId,
				grant_type: 'authorization_code',
				code: c,
				redirect_uri: redirectUri,
				code_verifier: localStorage.getItem('code_verifier'),
			}),
		}
		
		const url = new URL('https://accounts.spotify.com/api/token');
		const body = await fetch(url, payload);
		return await body.json();
	}

	getToken(code).then((response) => {
		localStorage.setItem('access_token', response.access_token);
		localStorage.setItem('refresh_token', response.refresh_token);

		window.location.href = 'http://fede-ai.github.io/Lyrics-viewer/success.html';
	}) 
}
//fail after sporify login
else {
	window.localStorage.setItem('error', error);
	window.location.href = 'http://fede-ai.github.io/Lyrics-viewer/fail.html';
}
