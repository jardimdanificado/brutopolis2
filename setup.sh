rm -rf build/*
cp -r lib build/

rm -rf bruter
rm include/bruter.h
git clone https://github.com/jardimdanificado/bruter -b experimental
cd bruter
if [ -n "$WEB" ]; then
    EMCC=emcc ./build.sh
    cd ..
    rm lib/bruter.js lib/bruter.wasm
    cp bruter/build/web/bruter.js lib/
    cp bruter/build/web/bruter.wasm lib/
elif [ -n "$BOTH" ]; then
    EMCC=emcc ./build.sh
    cd ..
    rm lib/bruter.js lib/bruter.wasm
    cp bruter/build/web/bruter.js lib/
    cp bruter/build/web/bruter.wasm lib/
    cd bruter
    ./build.sh
    cd ..
    cp bruter/build/lib/libbruter.a lib/
else
    ./build.sh
    cd ..
    cp bruter/build/lib/libbruter.a lib/
fi
cp bruter/include/bruter.h include/
cp data build/ -r