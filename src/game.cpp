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
    lastUpdateTime = gameBeginTime;

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

    for(const auto & db : deadBullets)
    {
        auto fb = bullets.find(db);
        if (fb != bullets.end())
        {
            bullets.erase(fb);
        }
    }

    deadBullets.clear();

    for(auto & p : players)
    {
        p.update();

        constrainPos(p.pos, p.size);
    }

    for(auto & b : bullets)
    {
        b.second.update(deltaTime);
        
        if (constrainPos(b.second.pos, b.second.size))
        {
            deadBullets.push_back(b.first);
        }

        for(auto & p : players)
        {
            if (bulletCollide(p, b.second))
            {
                deadBullets.push_back(b.first);
                // printf("Bullet %d (from player %d), color %d collided with player %d, color %d\n",
                //     b.first, b.second.playerId, b.second.colorId, p.id, p.colorId);
                p.colorId = b.second.colorId;
            }
        }
    }

    if (players.empty() || lastUpdateTime - gameBeginTime > roundLength)
    {
        printf("Players empty or time ran out! Finishing game...\n");
        finished = true;
    }
    else
    {
        bool sameColor = true;
        int col = players[0].colorId;
        for (auto & p : players)
        {
            //printf("Player %d color id %d\n", p.id, p.colorId);
            if (p.colorId != col)
            {
                sameColor = false;
                break;
            }
        }

        finished = sameColor;

        if (finished)
        {
            printf("All %d player colors the same! Finishing game...\n", players.size());
        }
    }

    lastUpdateTime = t;
}

bool Game::bulletCollide(const Player & p, const Bullet & b)
{
    if (p.colorId == b.colorId) return false;
    return glm::length(p.pos - b.pos) < (p.size + b.size);
}

Player * Game::addHuman()
{
    colors.push_back(genColor());
    addPlayer(numPlayers++, true, colors.size() - 1);
    return &players[players.size() - 1];
}

Player * Game::addHuman(glm::vec4 color)
{
    int colorId = 0;

    for(int i = 0; i < colors.size(); i++)
    {
        auto diff = glm::length(colors[i] - color);

        if (diff < colorTolerance)
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

    return &players[players.size() - 1];
}

void Game::addPlayer(int playerId, bool human, int colorId)
{
    printf("Adding %s %d with color id %d\n", human ? "humanoid" : "bot", numPlayers, colorId);
    players.push_back(Player(this, glm::linearRand(glm::vec2(0.0f), mapSize),
        playerId, human, colorId, playerSize, playerDonut));

    const auto & p = players[players.size() - 1];

    printf("player %d (color %d) spawned at %.2f, %.2f\n", p.id, p.colorId, p.pos.x, p.pos.y);
}

unsigned int Game::requestBullet(const Player * p)
{
    if (!p) return 0;

    auto timeDiff = lastUpdateTime - p->lastFireTime;
    if (timeDiff > p->fireRate)
    {
        bullets[numBullets++] = Bullet(p->headingPos, p->input.aim * 3.0f * p->maxSpeed, 
            p->id, p->colorId, p->size * 0.2f, lastUpdateTime);
        return lastUpdateTime;
    }

    return 0;
}