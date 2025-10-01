import webview
import base64
import hashlib
import secrets
import requests
import time
from screeninfo import get_monitors
import os
import subprocess

# === CONFIG ===
CLIENT_ID = "244ba241897d4c969d1260ad0c844f91"
REDIRECT_URI = "https://fede-ai.github.io/Lyrics-viewer/index.html"
SCOPES = "user-modify-playback-state user-read-playback-state user-read-currently-playing"

# === PKCE helpers ===
def generate_code_verifier():
	return base64.urlsafe_b64encode(secrets.token_bytes(32)).rstrip(b"=").decode("utf-8")

def generate_code_challenge(verifier):
	digest = hashlib.sha256(verifier.encode("utf-8")).digest()
	return base64.urlsafe_b64encode(digest).rstrip(b"=").decode("utf-8")

# === Globals ===
code_verifier = generate_code_verifier()
access_token = ""
refresh_token = ""

# === Build auth URL ===
auth_url = (
	"https://accounts.spotify.com/authorize"
	f"?client_id={CLIENT_ID}"
	"&response_type=code"
	f"&redirect_uri={REDIRECT_URI}"
	f"&scope={SCOPES}"
	"&code_challenge_method=S256"
	f"&code_challenge={generate_code_challenge(code_verifier)}"
)

window: webview.Window | None = None
class Api: 
	def log(self, code):
		token_url = "https://accounts.spotify.com/api/token"
		response = requests.post(
			token_url,
			data={
				"client_id": CLIENT_ID,
				"grant_type": "authorization_code",
				"code": code,
				"redirect_uri": REDIRECT_URI,
				"code_verifier": code_verifier
			}
		)
		tokens = response.json()

		global access_token, refresh_token
		access_token = tokens["access_token"]
		refresh_token = tokens.get("refresh_token")

		'''
		with open("tokens.txt", "w") as f:
			f.write(access_token + "\n")
			f.write((refresh_token or "") + "\n")

		profile = requests.get(
			"https://api.spotify.com/v1/me",
			headers={"Authorization": f"Bearer {access_token}"}
		).json()
		print(f"Login successful! Welcome, {profile.get('display_name', 'Spotify User')}")
		'''

		time.sleep(1.5)
		assert(window != None)
		window.destroy()

m = get_monitors()[0]
h = int(m.height / 1.5)
w = int(h * 0.85)
x = int(m.width / 2 - w / 2)
y = int(m.height / 2 - h / 2)

window = webview.create_window("Spotify Login", auth_url, x=x, y=y, width=w, height=h, on_top=True, js_api=Api())
assert(window != None)
webview.start(debug=False)

print("Starting overlay...")
path = os.path.dirname(os.path.realpath(__file__)) + "/build/Release/Lyrics-viewer.exe "

subprocess.run(
	path + access_token + " " + refresh_token, 
	creationflags=subprocess.CREATE_NO_WINDOW
)