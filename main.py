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

CLIENT_ID = "244ba241897d4c969d1260ad0c844f91"
REDIRECT_URI = "https://fede-ai.github.io/Lyrics-viewer/index.html"
SCOPES = "user-modify-playback-state user-read-playback-state user-read-currently-playing"

#PKCE helpers
def generate_code_verifier() -> str:
	return base64.urlsafe_b64encode(secrets.token_bytes(32)).rstrip(b"=").decode("utf-8")

def generate_code_challenge(verifier: str) -> str:
	digest = hashlib.sha256(verifier.encode("utf-8")).digest()
	return base64.urlsafe_b64encode(digest).rstrip(b"=").decode("utf-8")

code_verifier = generate_code_verifier()
access_token = ""
refresh_token = ""
window: webview.Window | None = None
exe_path = ""
tokens_dir = ""

#running as exe (release mode)
if getattr(sys, 'frozen', False):
	tokens_dir = os.path.dirname(sys.executable)
	exe_path = tokens_dir + "\\..\\Lyrics-viewer.exe"
#running as script (debug mode)
else:
	tokens_dir = os.path.dirname(os.path.abspath(__file__)) + "\\build\\Debug"
	exe_path = tokens_dir + "\\Lyrics-viewer.exe"

#build auth URL
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

		time.sleep(1)
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
	if getattr(sys, 'frozen', False):
		webview.start(debug=False)
	else:
		webview.start(debug=True)

def run_overlay() -> None:
	print(f"Starting overlay at path \"{exe_path}\"")

	if getattr(sys, 'frozen', False):
		subprocess.Popen(
			exe_path + " " + access_token + " " + refresh_token, 
			creationflags=subprocess.CREATE_NO_WINDOW
		)
	else:
		subprocess.run(
			exe_path + " " + access_token + " " + refresh_token
		)

#try fetching access token using refresh token or perform auth if necessary
def get_access_token() -> None:
	global access_token, refresh_token
	token_url = "https://accounts.spotify.com/api/token"
	response = requests.post(
		token_url,
		data={
			"client_id": CLIENT_ID,
			"grant_type": "refresh_token",
			"refresh_token": refresh_token
		}
	)

	if response.status_code != 200:
		print(response.text)
		perform_auth()
	else:
		tokens = response.json()
		access_token = tokens["access_token"]
		refresh_token = tokens["refresh_token"]

#open the file in read mode or create it if it doesn't exist
if not os.path.exists(tokens_dir + "\\token.txt"):
	open(tokens_dir + "\\token.txt", "w").close()
with open(tokens_dir + "\\token.txt", "r") as f:
	lines = f.readlines()
	if len(lines) >= 1:
		refresh_token = lines[0].strip()

if (refresh_token == ""):
	perform_auth()
else:
	get_access_token()

if access_token != "" and refresh_token != "":
	with open(tokens_dir + "\\token.txt", "w") as f:
		f.write(refresh_token + "\n")
	run_overlay()