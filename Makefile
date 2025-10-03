dev:
	python main.py

compile:
	pyinstaller --onedir --noconsole --noconfirm --distpath "build/Release" --name "Lyrics-viewer" main.py

run:
	./build/Release/Lyrics-viewer/Lyrics-viewer.exe