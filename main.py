import webview
import base64
import hashlib
import secrets
import requests
import time
from screeninfo import get_monitors
import os
import subprocess
import sys

# === CONFIG ===
CLIENT_ID = "244ba241897d4c969d1260ad0c844f91"
REDIRECT_URI = "https://fede-ai.github.io/Lyrics-viewer/index.html"
SCOPES = "user-modify-playback-state user-read-playback-state user-read-currently-playing"

# === PKCE helpers ===
def generate_code_verifier() -> str:
	return base64.urlsafe_b64encode(secrets.token_bytes(32)).rstrip(b"=").decode("utf-8")

def generate_code_challenge(verifier: str) -> str:
	digest = hashlib.sha256(verifier.encode("utf-8")).digest()
	return base64.urlsafe_b64encode(digest).rstrip(b"=").decode("utf-8")

# === Globals ===
code_verifier = generate_code_verifier()
access_token = ""
refresh_token = ""
window: webview.Window | None = None

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
		refresh_token = tokens["refresh_token"]

		'''
		with open("tokens.txt", "w") as f:
			f.write(access_token + "\n")
			f.write(refresh_token + "\n")

		_ = requests.get(
			"https://api.spotify.com/v1/me",
			headers={"Authorization": f"Bearer {access_token}"}
		).json()
		'''

		time.sleep(1.5)
		assert(window != None)
		window.destroy()

def perform_auth() -> None:
	m = get_monitors()[0]
	h = int(m.height / 1.5)
	w = int(h * 0.85)
	x = int(m.width / 2 - w / 2)
	y = int(m.height / 2 - h / 2)

	global window
	window = webview.create_window("Spotify Login", auth_url, x=x, y=y, width=w, height=h, on_top=True, js_api=Api())
	
	assert(window != None)
	webview.start(debug=False)

def run_overlay() -> None:
		#running as exe
	if getattr(sys, 'frozen', False):
		exe_dir = os.path.dirname(sys.executable) + "\\..\\.."
	#running as script
	else:
		exe_dir = os.path.dirname(os.path.abspath(__file__))

	exe_dir = exe_dir + "\\build\\Release\\Lyrics-viewer.exe"
	print(f"Starting overlay at path \"{exe_dir}\"...")

	#Popen (async) vs run (sync)
	subprocess.Popen(
		exe_dir + " " + access_token + " " + refresh_token, 
		creationflags=subprocess.CREATE_NO_WINDOW
	)

#TODO: try fetching tokens from cache
if access_token == "" or refresh_token == "":
	perform_auth()

if access_token != "" and refresh_token != "":
	run_overlay()