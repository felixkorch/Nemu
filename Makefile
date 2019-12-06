CC = em++

FLAGS = -O3 -fpermissive -std=c++17 -D NEMU_PLATFORM_WEB

EM_FLAGS = -s DEMANGLE_SUPPORT=1 \
-s WASM=1 -s USE_WEBGL2=1 \
-s DEMANGLE_SUPPORT=1 -s ASSERTIONS \
-s ALLOW_MEMORY_GROWTH=1 -s --memory-init-file 0 -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
-s USE_SDL=2 -s USE_SDL_TTF=2 -s EXPORTED_FUNCTIONS='["_UploadRom", "_main"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]'

INC = -I src -I dependencies/SDL2/include

TARGETS = emscripten/obj/Main.o

nemu: $(TARGETS)
	$(CC) $(FLAGS) $(EM_FLAGS) $(INC) $(TARGETS) -o emscripten/index.html --embed-file fonts --shell-file emscripten/layout.html

emscripten/obj/Main.o: src/Nemu/Main.cpp
	$(CC) $(FLAGS) $(EM_FLAGS) $(INC) -c src/Nemu/Main.cpp -o emscripten/obj/Main.o