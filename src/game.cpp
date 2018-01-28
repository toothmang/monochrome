#include "game.h"

#include "glm/gtc/random.hpp"

#include "SDL2/SDL.h"

#include <algorithm>

#include <stdio.h>

glm::vec4 genColor()
{
    glm::vec3 bc = glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f));
    return glm::vec4(bc, 1.0f);
}

void Game::begin()
{   
    gameBeginTime = SDL_GetTicks();

    printf("%s called! Game starting at tick %d\n", __FUNCTION__, gameBeginTime);

    // Generate bots
    for(int i = 0; i < numBots; i++)
    {
        bool badColor = true;
        glm::vec4 bc = genColor();

        // while (badColor)
        // {
        //     bc = genColor();
        //     badColor = std::any_of(colors.begin(), colors.end(), 
        //         [this, bc](const glm::vec4 & v)
        //         {
        //             return (bc - v).length() < colorTolerance;
        //         });
        // }

        colors.push_back(bc);

        addPlayer(numPlayers++, false, colors.size() - 1);
    }
}

void Game::update()
{
    auto t = SDL_GetTicks();

    deltaTime = (float)(t - lastUpdateTime) / 1000.0f;

    for(auto & player : players)
    {
        player.update();

        constrainPos(player.pos, player.size);
    }

    if (players.empty())
    {
        finished = true;
    }

    lastUpdateTime = t;
}

Player * Game::addHuman()
{
    colors.push_back(genColor());
    addPlayer(numPlayers++, true, colors.size() - 1);
    return &players.back();
}

Player * Game::addHuman(glm::vec4 color)
{
    int colorId = 0;

    for(int i = 0; i < colors.size(); i++)
    {
        auto diff = colors[i] - color;

        if (diff.length() < colorTolerance)
        {
            colorId = i;
            break;
        }
    }
    if (colorId == 0)
    {
        colors.push_back(color);
        colorId = colors.size() - 1;
    }

    addPlayer(numPlayers++, true, colorId);

    return &players.back();
}

void Game::addPlayer(int playerId, bool human, int colorId)
{
    printf("Adding %s %d with color id %d\n", human ? "humanoid" : "bot", numPlayers, colorId);
    players.push_back(Player(this, playerId, human, colorId));

    auto & p = players[players.size() - 1];

    p.pos = glm::linearRand(glm::vec2(0.0f), mapSize);

    printf("player %d spawned at %.2f, %.2f\n", p.id, p.pos.x, p.pos.y);
}