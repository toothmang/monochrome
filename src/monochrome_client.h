#pragma once

#include "SDL2/SDL.h"
//#include "SDL2/SDL_opengl.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include "imgui.h"

#include <vector>

class Game;
class Player;

class monochrome_client
{
    public:
        static monochrome_client & get()
        {
            static monochrome_client mc;
            return mc;
        }

        enum State
        {
            LoadingScreen,
            Playing,
            Victory
        };

        State state;

        char playerName[1024] = "Jimothy";
        ImVec4 playerColor;
        bool usePlayerColor = false;

        Player * player = nullptr;
        Game * game = nullptr;

        int width = 1280;
        int height = 720;

        SDL_Window * window = nullptr;
        SDL_Renderer * renderer = nullptr;
        SDL_GameController * controller = nullptr;
        SDL_Joystick * joystick = nullptr;
        SDL_GLContext glcontext;
        SDL_DisplayMode current;

        bool running = false;

        bool init();

        void shutdown();

        virtual ~monochrome_client()
        {
            shutdown();
        }

        // Main update loop
        void update();

        void updateState(State newState);

        // update function for loading screen state
        void updateLoading();
        // update function for gameplay update
        void updatePlaying();
        // update function for victory screen update
        void updateVictory();

        // renders game and ui
        void render();

        // renders top menu bar
        void renderTopMenu();

        void renderLoading();

        void renderPlayer(const Player & p);

        // renders game if playing
        void renderGame();

        void renderVictory();

        // Creates a new game
        void newgame();
        
        

    protected:
        monochrome_client() {}
};