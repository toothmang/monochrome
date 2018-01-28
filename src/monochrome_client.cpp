#include "monochrome_client.h"

#include "game.h"
#include "player.h"

#include "imgui_impl_sdl.h"

#include <stdio.h>


ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

bool monochrome_client::init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GetCurrentDisplayMode(0, &current);
    
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE , &window, &renderer);
    SDL_SetWindowTitle(window, "Monochrome - Winning Colors!");
    glcontext = SDL_GL_CreateContext(window);

    if (!ImGui_ImplSdl_Init(window))
    {
        printf("imgui failed to init!\n");
        shutdown();
        return false;
    }

    SDL_GameControllerEventState(SDL_QUERY);
    SDL_JoystickEventState(SDL_QUERY);

    var numJoysticks = SDL_NumJoysticks();

    //Check for joysticks
    if( numJoysticks < 1 )
    {
        printf( "Warning: No joysticks connected!\n" );
    }
    else
    {
        for(int i = 0; i < numJoysticks; i++)
        {
            if (SDL_IsGameController(i))
            {
                printf("Using controller index %i, name %s\n", i, SDL_GameControllerNameForIndex(i));
                controller = SDL_GameControllerOpen(i);
                joystick = SDL_GameControllerGetJoystick(controller);
                break;
            }
        }
    }

    if( controller == nullptr || joystick == nullptr )
    {
        printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
    }

    return true;
}

 void monochrome_client::shutdown()
{
    if (!running) return;

    running = false;
    if (joystick)
    {
        SDL_JoystickClose( joystick );
        joystick = nullptr;
    }

    ImGui_ImplSdl_Shutdown();
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void monochrome_client::update()
{
    switch(state)
    {
        case LoadingScreen:
            loadingScreen();
            break;
        case Playing:
            playing();
            break;
        case Victory:
            victory();
            break;
        default:
            break;
    }
}

void monochrome_client::updateLoading()
{
    ImGui::Begin("LOADING SCREEN");
    ImGui::InputText("Enter name", buf, 1024);
    ImGui::ColorEdit3("Choose color", (float*)&color);

    if (ImGui::Button("PLAY"))
    {
        )
        state = Playing;
    }

    ImGui::NewLine();

    ImGui::Text("Left stick to move, right stick to aim, any button to fire");

    ImGui::End();
}

void monochrome_client::updatePlaying()
{
    game->update();

    if (game->finished)
    {
        state = Victory;
    }
}

void monochrome_client::updateVictory()
{
    ImGui::Begin("Victory screen!");
    
    if (ImGui::Button("End"))
    {
        state = LoadingScreen;
    }
    ImGui::End();
}

void monochrome_client::render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    renderTopMenu();

    switch(state)
    {
        case LoadingScreen:
            renderLoading();
            break;
        case Playing:
            renderGame();
            break;
        case Victory:
            renderVictory();
            break;
        default:
            break;
    }
}

void monochrome_client::renderTopMenu()
{
    ImGui::BeginMainMenuBar();

    ImGui::Text("Monochrome! Application average %.3f ms/frame (%.1f FPS)", 
        1000.0f / ImGui::GetIO().Framerate,
        ImGui::GetIO().Framerate);

    ImGui::EndMainMenuBar();
}

void monochrome_client::renderGame()
{
    //SDL_Surface *screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
    // boxColor(screen, 0, 0, width, height, 0xff);

    // boxColor(screen, 0, 0, 98, 98, 0xff0000ff);
    // boxRGBA(screen, 100, 0, 198, 98, 0, 0, 0xff, 0xff);
    // // check that the x2 > x1 case is handled correctly
    // boxColor(screen, 298, 98, 200, 0, 0x00ff00ff);
    // boxColor(screen, 398, 98, 300, 0, 0xff0000ff);

    // rectangleColor(screen, 0, 100, 98, 198, 0x000ffff);
    // rectangleRGBA(screen, 100, 100, 198, 198, 0xff, 0, 0, 0xff);

    // ellipseColor(screen, 300, 150, 99, 49, 0x00ff00ff);
    // filledEllipseColor(screen, 100, 250, 99, 49, 0x00ff00ff);
    // filledEllipseRGBA(screen, 250, 300, 49, 99, 0, 0, 0xff, 0xff);

    // lineColor(screen, 300, 200, 400, 300, 0x00ff00ff);
    // lineRGBA(screen, 300, 300, 400, 400, 0, 0xff, 0, 0xff);

    // SDL_UpdateRect(screen, 0, 0, 0, 0);
}

void monochrome_client::renderVictory()
{

}

void monochrome_client::newgame()
{
    if (game)
    {
        delete game;
        game = new Game();
        player = game->addPlayer(true, glm::vec4(playerColor.x, playerColor.y, playerColor.z, playerColor.w));
    }
}
bool done = false;

void mainLoop();

int main(int argc, char **argv) 
{
    if (!monochrome_client::get().init())
    {
        printf("Failed to initialize client! Shutting down\n");
        return -1;
    }

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while (!done)
    {
        mainLoop();
        //SDL_Delay(16);
    }
#endif
    return 0;
}

void mainLoop()
{
    auto & client = monochrome_client::get();

    // Main loop
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSdl_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
        {
            done = true;
            return;
        }
    }
    ImGui_ImplSdl_NewFrame(window);

    client.update();

    // Rendering
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    SDL_GL_SwapWindow(window);
}