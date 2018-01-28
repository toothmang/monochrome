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
            break;
        case Victory:
            victoryTime = SDL_GetTicks();
            break;
        default:
            break;
    }
}

void monochrome_client::newgame()
{
    if (game)
    {
        delete game;
        game = nullptr;
    }
    gs.size = glm::vec2(width, height);
    game = new Game(gs);
    if (gs.usePlayerColor)
    {
        player = game->addHuman(gs.playerColor);
    }
    else
    {
        player = game->addHuman();
    }

    game->begin();
}

void monochrome_client::updateLoading()
{
    ImGui::Begin("LOADING SCREEN");
    ImGui::InputText("Enter name", playerName, 1024);
    
    ImGui::Checkbox("Custom color", &gs.usePlayerColor);

    if (gs.usePlayerColor)
    {
        ImGui::ColorEdit3("Choose color", (float*)&gs.playerColor);
    }

    ImGui::InputInt("Num bots", &gs.numBots);

    ImGui::InputFloat("Player size", &gs.playerSize);
    ImGui::InputFloat("Player donut", &gs.playerDonut);

    ImGui::InputFloat("Player max speed", &Player::maxSpeed);
    int fr = Player::fireRate;
    if (ImGui::InputInt("Player fire rate", &fr))
    {
        Player::fireRate = (unsigned int)fr;
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
    auto victoryElapsed = SDL_GetTicks() - victoryTime;
    victoryRemaining = victoryLength - victoryElapsed;

    if (victoryElapsed > victoryLength)
    {
        updateState(LoadingScreen);
    }
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

    //SDL_RenderPresent(renderer);
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

}

void monochrome_client::renderGame()
{
    auto & cr = CircleRenderer::get();

    static float playerMinRadius = 18.0f;
    static float offsetScale = 0.4f;
    
    for(const auto & p : game->players)
    {
        // Render a sphere for the player position
        cr.render(p.pos, game->colors[p.colorId], p.size, p.minSize);

        // And also render a smaller sphere for their heading
        cr.render(p.headingPos, game->colors[p.colorId], p.size * offsetScale);

        // If human, highlight them a bit
        if (p.isHuman)
        {
            cr.render(p.pos, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), p.size * 1.66f, p.size * 1.5f);
        }
    }

    for(const auto & b : game->bullets)
    {
        cr.render(b.second.pos, game->colors[b.second.colorId], b.second.size);
    }

    ImGui::Begin("PLAYTIME!");

    auto diff = game->lastUpdateTime - game->gameBeginTime;
    auto rem = (game->roundLength - diff) * 0.001f;

    ImGui::Text("Time remaining: %.2f sec", rem);

    ImGui::SliderFloat("Offset scale", &offsetScale, 0.0f, 1.0f);

    for(auto & p : game->players)
    {
        if (p.isHuman)
        {
            ImGui::Text("Player pos: %.2f, %.2f. Heading: %.2f, %.2f", p.pos.x, p.pos.y, 
                p.input.aim.x, p.input.aim.y);
            
        }
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
    ImGui::Begin("Victory screen!");

    ImGui::Text("Time to next round: %.2f", victoryRemaining * 0.001f);
    
    if (ImGui::Button("End"))
    {
        updateState(LoadingScreen);
    }
    ImGui::End();
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