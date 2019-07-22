CC = em++

FLAGS = -O3 -fpermissive -std=c++17 -D NEMU_PLATFORM_WEB

EM_FLAGS = -s FULL_ES3=1 -s USE_GLFW=3 -s DEMANGLE_SUPPORT=1 \
-s WASM=1 -s USE_WEBGL2=1 -lglfw3 -lGL \
-s DEMANGLE_SUPPORT=1 -s ASSERTIONS \
-s ALLOW_MEMORY_GROWTH=1 -s --memory-init-file 0 -s ERROR_ON_UNDEFINED_SYMBOLS=0

INC = -I ext -I src -I ext/Glad -I ext/glm -I ext/GLFW/include

TARGETS = emscripten/obj/Main.o

nemu: $(TARGETS)
	$(CC) $(FLAGS) $(EM_FLAGS) $(INC) $(TARGETS) -o emscripten/index.html # --shell-file emscripten/layout.html

emscripten/obj/Main.o: src/Nemu/Main.cpp
	$(CC) $(FLAGS) $(EM_FLAGS) $(INC) -c src/Nemu/Main.cpp -o emscripten/obj/Main.o