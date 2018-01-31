#include "game.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"

#include "SDL2/SDL.h"

#include <algorithm>

#include <stdio.h>

// Given hue,sat,val in [0,1], return r,g,b in [0,1] as a vec3
glm::vec3 hsv2rgb(float hue, float sat, float val) {
    float c = val * sat;
    float m = val - c;

    int   phase   = hue * 6.0; // hue / 60 works if you want hue in [0,360)
    float phase_r = hue * 6.0 - phase;

    glm::vec3 ret(m);

    if (phase == 6 ||
        phase == 0) { ret.r += c;  ret.g +=       phase_r  * c; }
    if (phase == 1) { ret.g += c;  ret.r += (1. - phase_r) * c; }
    if (phase == 2) { ret.g += c;  ret.b +=       phase_r  * c; }
    if (phase == 3) { ret.b += c;  ret.g += (1. - phase_r) * c; }
    if (phase == 4) { ret.b += c;  ret.r +=       phase_r  * c; }
    if (phase == 5) { ret.r += c;  ret.b += (1. - phase_r) * c; }

    return ret;
}

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

	generateTerrain();

    // Generate bots
    for(int i = 0; i < numBots; i++)
    {
        bool badColor = true;

        float h = glm::linearRand(0.0f, 1.0f);
        float s = glm::linearRand(0.0f, 1.0f);
        float v = glm::linearRand(0.3f, 1.0f);
        glm::vec4 bc = glm::vec4(hsv2rgb(h, s, v), 1.0f);

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

	auto areaPerPlayer = mapArea() / players.size();

	auto sideArea = sqrt(areaPerPlayer);

	auto numCols = mapSize.x / sideArea;
	auto numRows = mapSize.y / sideArea;

	auto sqr = glm::vec2(sideArea);

	int pi = 0;
	for (int i = 0; i < numRows; i++)
	{
		float y = i * sideArea;
		for (int j = 0; j < numCols; j++)
		{
			float x = j * sideArea;

			auto offset = glm::linearRand(glm::vec2(0.0f), sqr);

			players[pi++].pos = glm::vec2(x, y) + offset;
		}
	}
}

void Game::generateTerrain()
{
	auto ma = mapArea();

	auto pa = (numBots + 4) * glm::pow(playerSize * 0.5f, 2.0f) * glm::pi<float>();

	auto ta = (ma - pa) * 0.03f;

	auto tp = ta / ma;

	printf("Generating terrain as %.2f percent of map\n", tp);

	int numTerrains = glm::linearRand(0, 100);

	float areaPerTerrain = glm::sqrt(ta / numTerrains);

	for (int i = 0; i < numTerrains; i++)
	{
		terrain.push_back(glm::vec4(
			glm::vec3(glm::linearRand(glm::vec2(0.0f), mapSize), glm::linearRand(areaPerTerrain * 0.5f, areaPerTerrain * 1.5f)),
			glm::linearRand(100.0f, 200.0f)));
	}
}

void Game::end()
{
    gameEndTime = SDL_GetTicks();

    finished = true;

    victorStats.clear();

	for (auto & p : players)
	{
		if (p.stats.bulletsFired > stats.players.bulletsFired)
		{
			stats.players.bulletsFired = p.stats.bulletsFired;
		}
		if (p.stats.highestHealth > stats.players.highestHealth)
		{
			stats.players.highestHealth = p.stats.highestHealth;
		}
		if (p.stats.numConverted > stats.players.numConverted)
		{
			stats.players.numConverted = p.stats.numConverted;
		}
		if (p.stats.teamSwitches > stats.players.teamSwitches)
		{
			stats.players.teamSwitches = p.stats.teamSwitches;
		}
	}


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
			if (glm::linearRand(0.0f, 1.0f) > 0.75f)
			{
				auto bpos = b.second.pos;

				if (bpos.x == 0.0f || bpos.x == mapSize.x)
				{
					b.second.vel.x = -b.second.vel.x;
				}
				else if (bpos.y == 0.0f || bpos.x == mapSize.y)
				{
					b.second.vel.y = -b.second.vel.y;
				}
			}
			else
			{
				deadBullets.push_back(b.first);
				continue;
			}
        }

		glm::vec3 b3(b.second.pos, b.second.size);
		for (auto & t : terrain)
		{
			if (collides(t, b3))
			{
				t.z -= 1;

				if (t.z < 0.0f)
				{
					t = glm::vec4(0.0f);
				}
				deadBullets.push_back(b.first);
				continue;
			}
		}

        for(auto & p : players)
        {
            if (bulletCollide(p, b.second) && !finished)
            {
                deadBullets.push_back(b.first);
				if (p.changeColor(b.second.colorId))
				{
					players[b.second.playerId].stats.numConverted++;
				}
				continue;
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

bool Game::collides(const glm::vec3 & s1, const glm::vec3 & s2)
{
	return glm::length(s1.xy - s2.xy) < (s1.z + s2.z);
}


bool Game::bulletCollide(const Player & p, const Bullet & b)
{
	if (p.colorId == b.colorId && p.id == b.playerId) return false;
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