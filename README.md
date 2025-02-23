# Lyrics-viewer
This is a small windows-only project that, using the Spotify API, allows you to always see the lyrics of the song you are listening to.

## Dependencies:
- SFML 2.6.1 (https://www.sfml-dev.org/download/sfml/2.6.1/)
- libcurl 8.12.1 (https://curl.se/windows/)
- CEF 131.2.7 (https://cef-builds.spotifycdn.com/index.html)

<!---
1) download needed dependencies
2) rename "libcurl.dll.a" to "libcurl.lib"
3) build and compile (release and debug) libcef_dll_wrapper
4) move the wrapper to the Release/Debug folders
5) build and compile Lyrics-viewer
-->