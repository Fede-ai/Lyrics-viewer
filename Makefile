dev:
	python main.py

clean:
	rmdir /S /Q "dist/Lyrics-viewer"

compile:
	pyinstaller --onedir --noconsole --distpath "dist" --name "Lyrics-viewer" main.py

run:
	./dist/Lyrics-viewer/Lyrics-viewer.exe