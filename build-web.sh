rm -rf build/*
cp -r lib build/

RLPATH=./lib/web/libraylib.a

if [ -n "$SETUP" ]; then
	rm -rf bruter
	rm include/bruter.h
	git clone https://github.com/jardimdanificado/bruter -b experimental
	cd bruter
	EMCC=emcc ./build.sh
	cd ..
	rm lib/bruter.js lib/bruter.wasm
	cp bruter/build/web/bruter.js lib/
	cp bruter/build/web/bruter.wasm lib/
	cp bruter/include/bruter.h include/
	rm -rf bruter
fi

emcc -o build/index.html src/main.c -Llib/web -Iinclude -lbruter -lraylib -s USE_GLFW=3 -s ASYNCIFY --shell-file src/minshell.html --preload-file data