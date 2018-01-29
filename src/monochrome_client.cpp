#include "monochrome_client.h"

#include "game.h"
#include "player.h"

#include "imgui_impl_sdl.h"

#include "glm/gtc/constants.hpp"

#include "circlerenderer.h"

#include <stdio.h>
<<<<<<< Updated upstream
#include <algorithm>
#include <functional>
=======

#include <SDL2/SDL_opengles2.h>
>>>>>>> Stashed changes

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

    fbo = new FBO(width, height);
    fbo->bind();
    fbo->blank();
    fbo->unbind();

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
            printf("Starting to play!\n");
            newgame();
            break;
        case Victory:
            printf("Going to victory!!\n");
            game->end();
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
        humanPlayer = game->addHuman(gs.playerColor);
    }
    else
    {
        humanPlayer = game->addHuman();
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
    }

    ImGui::NewLine();

    ImGui::Text("Move: left stick / WSAD");
    ImGui::Text("Aim and fire: right stick (just move it!) or left mouse (move and click)");

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
    game->update();

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
            renderPlaying();
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

void monochrome_client::renderPlaying()
{
    auto & cr = CircleRenderer::get();

    static float offsetScale = 0.4f;

    fbo->bind();

    for(const auto & p : game->players)
    {
        glm::vec4 color(0., 0., 0., 1.);
        if (p.colorId < game->colors.size()) color = game->colors[p.colorId];

        // Render a sphere for the player position
        cr.render(p.pos, color, p.size, p.minSize);

        // And also render a smaller sphere for their heading
        cr.render(p.headingPos, color, p.size * offsetScale);
    }

    fbo->unbind();
    fbo->load();

    //cr.render(glm::vec2(640., 360.), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1200., 0., true);

    glBlitFramebuffer(0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    fbo->unbind();


    for(const auto & p : game->players)
    {
        glm::vec4 color(0., 0., 0., 1.);
        if (p.colorId < game->colors.size()) color = game->colors[p.colorId];

        // Render a sphere for the player position
        cr.render(p.pos, color, p.size, p.minSize);

        // And also render a smaller sphere for their heading
        cr.render(p.headingPos, color, p.size * offsetScale);

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

    if (!game->finished)
    {
        ImGui::Begin("PLAYTIME!");

        auto diff = game->lastUpdateTime - game->gameBeginTime;
        auto rem = (game->roundLength - diff) * 0.001f;

        ImGui::Text("Time remaining: %.2f sec", rem);

        ImGui::SliderFloat("Offset scale", &offsetScale, 0.0f, 1.0f);

        if (humanPlayer >= 0 && humanPlayer < game->players.size())
        {
            auto hp = &game->players[humanPlayer];

            ImGui::Text("Player pos: %.2f, %.2f. Heading: %.2f, %.2f", hp->pos.x, hp->pos.y, 
                hp->input.aim.x, hp->input.aim.y);
        }
        

        // for(auto & p : game->players)
        // {
        //     if (p.isHuman)
        //     {
        //         ImGui::Text("Player pos: %.2f, %.2f. Heading: %.2f, %.2f", player->pos.x, player->pos.y, 
        //             player->input.aim.x, player->input.aim.y);
        //         break;
        //     }
        // }

        if (ImGui::Button("STAHP"))
        {
            updateState(Victory);
        }

        if (ImGui::CollapsingHeader("Stats"))
        {
            int numBullets = game->bullets.size();

            ImGui::Text("Bullet count: %d", numBullets);

            // Try to estimate memory usage?
            auto memUsage = game->getSize();

            auto mb = (double)memUsage / (1024 * 1024);

            ImGui::Text("Memory usage: %.2f MB", mb);


            auto stats = game->colorStats;

            std::sort(stats.begin(), stats.end(), [](const Game::ColorStat &a, const Game::ColorStat & b)
            {
                if (a.players == b.players)
                {
                    return a.index > b.index;
                }
                else
                {
                    return a.players > b.players;
                }
            });

            for(auto & s : stats)
            {
                if (s.players == 0) break;
                ImGui::Text("Color %d: Players %d, Center %.2f, %.2f",
                    s.index, s.players, s.avgPos.x, s.avgPos.y);
            }
        }

        ImGui::End();
    }
}

void monochrome_client::renderVictory()
{
    renderPlaying();

    ImGui::Begin("Victory screen!");

    ImGui::Text("Match length: %.2f", (game->gameEndTime - game->gameBeginTime) * 0.001f);
    ImGui::Text("Time to next round: %.2f", victoryRemaining * 0.001f);

    if (ImGui::Button("End"))
    {
        updateState(LoadingScreen);
    }

    // ImGui::PlotLines("Round plot", (float*)game->victorStats.data(), (int)game->victorStats.size(),
    //     sizeof(unsigned int) + sizeof(int), nullptr, FLT_MAX, FLT_MAX, ImVec2(500, 500), sizeof(Game::ColorStat));

    ImGui::NewLine();
    ImGui::Text("Winning team population over time");
    ImGui::PlotLines("", 
        [](void *data, int idx)
        {
            if (auto vs = (Game::ColorStat*)data)
            {
                return (float)vs[idx].players;
            }
            return 0.0f;
        }, 
        (void*)game->victorStats.data(), (int)game->victorStats.size(), 0, nullptr, FLT_MAX, FLT_MAX, 
        ImVec2(500, 500));

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

    client.lastInput = client.input;

    client.input.keysUp.clear();
    client.input.keysDown.clear();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSdl_ProcessEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                done = true;
                break;
            case SDL_KEYDOWN:
                client.input.keyboardState[event.key.keysym.sym] = true;
                client.input.keysDown.insert(event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                client.input.keyboardState[event.key.keysym.sym] = false;
                client.input.keysUp.insert(event.key.keysym.sym);
                break;
            case SDL_MOUSEMOTION:
                client.input.mousePos = glm::vec2(event.motion.x, event.motion.y);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                client.input.mouseState[event.button.button] = event.button.state;
                break;
            default:
                break;
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
