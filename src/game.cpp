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

    colorStats.resize(colors.size());
}

void Game::end()
{
    gameEndTime = SDL_GetTicks();

    finished = true;

    victorStats.clear();

    for(auto & ms : matchStats)
    {
        victorStats.push_back(ms[winningColorId]);
    }
}

void Game::update()
{
    if (finished) return;

    auto t = SDL_GetTicks();
    auto dt = t - lastUpdateTime;

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
            if (bulletCollide(p, b.second) && !finished)
            {
                deadBullets.push_back(b.first);
                p.colorId = b.second.colorId;
            }
        }
    }

    if ((t - lastStatTime) > statInterval)
    {
        for(int i = 0; i < colorStats.size(); i++)
        {
            auto & cs = colorStats[i];
            cs.time = t;
            cs.index = i;
            cs.players = 0;
            cs.avgPos = {0.0f, 0.0f};
        }

        for(auto & p : players)
        {
            auto & cs = colorStats[p.colorId];

            cs.players++;
            cs.avgPos += p.pos;
        }

        for(auto & cs : colorStats)
        {
            cs.avgPos = cs.avgPos * (1.0f / cs.players);
        }
        matchStats.push_back(colorStats);

        lastStatTime = t;
    }

    

    if (players.empty() || lastUpdateTime - gameBeginTime > roundLength)
    {
        printf("Players empty or time ran out! Finishing game...\n");
        finished = true;
    }
    else if (!finished)
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
            winningColorId = col;
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

int Game::addHuman()
{
    colors.push_back(genColor());
    addPlayer(numPlayers++, true, colors.size() - 1);
    return players.size() - 1;
}

int Game::addHuman(glm::vec4 color)
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

    return players.size() - 1;
}

void Game::addPlayer(int playerId, bool human, int colorId)
{
    players.push_back(Player(this, glm::linearRand(glm::vec2(0.0f), mapSize),
        playerId, human, colorId, playerSize, playerDonut));

    const auto & p = players[players.size() - 1];

    printf("player %d (color %d) spawned at %.2f, %.2f\n", p.id, p.colorId, p.pos.x, p.pos.y);
}

unsigned int Game::requestBullet(const Player * p)
{
    if (!p || finished || bullets.size() >= maxBullets) return 0;

    auto timeDiff = lastUpdateTime - p->lastFireTime;
    if (timeDiff > p->fireRate)
    {
        bullets[numBullets++] = Bullet(p->headingPos, p->input.aim * 3.0f * p->maxSpeed, 
            p->id, p->colorId, p->size * 0.2f, lastUpdateTime);
        return lastUpdateTime;
    }

    return 0;
}

size_t Game::getSize()
{
    return (sizeof(Bullet) * bullets.size())
        + (sizeof(int) * deadBullets.size())
        + (sizeof(Player) * players.size())
        + (sizeof(glm::vec4) * colors.size())
        + (sizeof(ColorStat) * players.size() * matchStats.size())
        + (sizeof(ColorStat) * colorStats.size())
        + (sizeof(ColorStat) * victorStats.size());

}