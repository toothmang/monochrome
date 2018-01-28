#include "game.h"

#include "glm/gtc/random.hpp"

#include "SDL2/SDL.h"

#include <algorithm>

void Game::begin()
{
    gameBeginTime = SDL_GetTicks();

    auto genColor = []()
    {
        glm::vec3 bc = glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f));
        return glm::vec4(bc, 1.0f);
    };

    // Generate bot colors
    for(int i = 0; i < numBots; i++)
    {
        bool badColor = true;
        glm::vec4 bc;

        while (badColor)
        {
            bc = genColor();
            badColor = std::any_of(colors.begin(), colors.end(), 
                [this, bc](const glm::vec4 & v)
                {
                    return (bc - v).length() < colorTolerance;
                });
        }

        colors.push_back(bc);

        addPlayer(false, colors[i]);
    }
}

void Game::update()
{
    for(auto & player : players)
    {
        player.update();

        constrainPos(player.pos);
    }

    if (players.empty())
    {
        finished = true;
    }
}

void Game::addBot(int colorId)
{
    players.push_back(Player(numPlayers++, false, colorId));
}

Player * Game::addPlayer(glm::vec4 color)
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

    players.push_back(Player(numPlayers++, true, colorId));

    return &players.back();
}