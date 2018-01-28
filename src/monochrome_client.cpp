#include "monochrome_client.h"

#include "game.h"
#include "player.h"

#include "imgui_impl_sdl.h"

#include "glm/gtc/constants.hpp"

#include "circlerenderer.h"

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

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (!ImGui_ImplSdl_Init(window))
    {
        printf("imgui failed to init!\n");
        shutdown();
        return false;
    }

    SDL_GameControllerEventState(SDL_QUERY);
    SDL_JoystickEventState(SDL_QUERY);

    int numJoysticks = SDL_NumJoysticks();

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
            updateLoading();
            break;
        case Playing:
            updatePlaying();
            break;
        case Victory:
            updateVictory();
            break;
        default:
            break;
    }
}

void monochrome_client::updateState(State newState)
{
    if (state == newState) return;

    state = newState;

    switch(state)
    {
        case LoadingScreen:
            break;
        case Playing:
            newgame();
            game->begin();
            break;
        case Victory:
            break;
        default:
            break;
    }

    
}

void monochrome_client::updateLoading()
{
    ImGui::Begin("LOADING SCREEN");
    ImGui::InputText("Enter name", playerName, 1024);
    if (ImGui::ColorEdit3("Choose color", (float*)&playerColor))
    {
        usePlayerColor = true;
    }

    if (ImGui::Button("PLAY"))
    {
        updateState(Playing);
        printf("Starting to play!\n");
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
        updateState(Victory);
    }
}

void monochrome_client::updateVictory()
{
    ImGui::Begin("Victory screen!");
    
    if (ImGui::Button("End"))
    {
        updateState(LoadingScreen);
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

    SDL_RenderPresent(renderer);
}

void monochrome_client::renderTopMenu()
{
    ImGui::BeginMainMenuBar();

    ImGui::Text("Monochrome! Application average %.3f ms/frame (%.1f FPS)", 
        1000.0f / ImGui::GetIO().Framerate,
        ImGui::GetIO().Framerate);

    ImGui::EndMainMenuBar();
}

void monochrome_client::renderLoading()
{
    int w, h, isfs;
    emscripten_get_canvas_size(&w, &h, &isfs);
    SDL_Rect rect = { 0, 0, w, h };
    SDL_RenderFillRect(renderer, &rect);
}

void monochrome_client::renderPlayer(const Player & player)
{


    static auto pi  = glm::pi<float>();
    static auto pih = glm::half_pi<float>();

    SDL_SetRenderDrawColor(renderer, 
        255, 
        255,
        255, 
        SDL_ALPHA_OPAQUE);

    //drew  28 lines with   4x4  circle with precision of 150 0ms
    //drew 132 lines with  25x14 circle with precision of 150 0ms
    //drew 152 lines with 100x50 circle with precision of 150 3ms
    const int prec = 27; // precision value; value of 1 will draw a diamond, 27 makes pretty smooth circles.
    float theta = 0;     // angle that will be increased each loop

    //starting point
    int ps = player.size;
    int psh = ps / 2;
    int x  = (float)ps * glm::cos(theta);//start point
    int y  = (float)ps * glm::sin(theta);//start point
    int x0 = player.pos.x;
    int y0 = player.pos.y;
    int x1 = x;
    int y1 = y;

    //printf("%d %d          %d %d\n", x0, y0, x, y);

    static SDL_Rect playerRect;

    playerRect.x = x0 - ps;
    playerRect.y = y0 - ps;
    playerRect.w = ps;
    playerRect.h = ps;

    SDL_RenderFillRect(renderer, &playerRect);

    return;

    //repeat until theta >= 90;
    float step = pih/(float)prec; // amount to add to theta each time (degrees)
    for(theta=step;  theta <= pih;  theta+=step)//step through only a 90 arc (1 quadrant)
    {
        //get new point location
        x1 = (float)player.size * glm::cos(theta) + 0.5; //new point (+.5 is a quick rounding method)
        y1 = (float)player.size * glm::sin(theta) + 0.5; //new point (+.5 is a quick rounding method)

        //draw line from previous point to new point, ONLY if point incremented
        if( (x != x1) || (y != y1) )//only draw if coordinate changed
        {
            SDL_RenderDrawLine(renderer, x0 + x, y0 - y,    x0 + x1, y0 - y1 );//quadrant TR
            SDL_RenderDrawLine(renderer, x0 - x, y0 - y,    x0 - x1, y0 - y1 );//quadrant TL
            SDL_RenderDrawLine(renderer, x0 - x, y0 + y,    x0 - x1, y0 + y1 );//quadrant BL
            SDL_RenderDrawLine(renderer, x0 + x, y0 + y,    x0 + x1, y0 + y1 );//quadrant BR
        }
        //save previous points
        x = x1;//save new previous point
        y = y1;//save new previous point
    }
    //arc did not finish because of rounding, so finish the arc
    if(x!=0)
    {
        x=0;
        SDL_RenderDrawLine(renderer, x0 + x, y0 - y,    x0 + x1, y0 - y1 );//quadrant TR
        SDL_RenderDrawLine(renderer, x0 - x, y0 - y,    x0 - x1, y0 - y1 );//quadrant TL
        SDL_RenderDrawLine(renderer, x0 - x, y0 + y,    x0 - x1, y0 + y1 );//quadrant BL
        SDL_RenderDrawLine(renderer, x0 + x, y0 + y,    x0 + x1, y0 + y1 );//quadrant BR
    }
}

void monochrome_client::renderGame()
{
    SDL_SetRenderDrawColor(renderer, 
        255, 
        255,
        255, 
        255);

    auto & cr = CircleRenderer::get();
    
    for(const auto & player : game->players)
    {
        cr.render(player.pos, player.size, game->colors[player.colorId]);
        //renderPlayer(player);
    }


    ImGui::Begin("PLAYTIME!");

    if (ImGui::CollapsingHeader("Stats"))
    {
        ImGui::Text("Player position: %.2f, %.2f", player->pos.x, player->pos.y);
    }

    if (ImGui::Button("STAHP"))
    {
        updateState(Victory);
        printf("Going to victory!!\n");
    }

    ImGui::End();
}

void monochrome_client::renderVictory()
{

}

void monochrome_client::newgame()
{
    if (game)
    {
        delete game;
        game = nullptr;
    }
    game = new Game(glm::vec2(width, height));
    if (usePlayerColor)
    {
        player = game->addHuman(glm::vec4(playerColor.x, playerColor.y, playerColor.z, playerColor.w));
    }
    else
    {
        player = game->addHuman();
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
    printf("emscripten setting main loop!\n");
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while (!done)
    {
        mainLoop();
        SDL_Delay(1);
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
    ImGui_ImplSdl_NewFrame(client.window);

    client.update();

    client.render();

    // Rendering
    // glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    // glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    // glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    SDL_GL_SwapWindow(client.window);
}