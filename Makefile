CC = em++
OL = -O2
CPPVER = -std=c++14

clean:
	-rm obj/*.bc
	-rm build/*.html*
	-rm build/*.js

obj/imgui.bc: src/imgui.cpp src/imgui_draw.cpp src/imgui_impl_sdl.cpp
	$(CC) src/imgui.cpp src/imgui_draw.cpp src/imgui_impl_sdl.cpp $(OL) $(CPPVER) -s USE_SDL=2 -o obj/imgui.bc
client: src/monochrome_client.cpp src/player.cpp src/game.cpp src/circlerenderer.cpp obj/imgui.bc
	$(CC) src/monochrome_client.cpp src/player.cpp src/game.cpp src/circlerenderer.cpp obj/imgui.bc $(OL) $(CPPVER) -s USE_SDL=2 -o build/monochrome.html

all: client


# -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]'
