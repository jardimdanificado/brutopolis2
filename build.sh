rm -rf build/data
cp -r data build/data
gcc -o build/brutopolis2 src/main.c -Llib -Iinclude -lbruter -lraylib -lGL -lm -lpthread -ldl -lrt -lX11