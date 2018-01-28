#pragma once

#include "player.h"
#include "bullet.h"
#include "gamestart.h"
#include "messages.hpp"

#include "glm/vec2.hpp"
#include <vector>
#include <map>

class Game
{
    public:
        glm::vec2 mapSize;
        std::vector<Player> players;
        std::vector<glm::vec4> colors;
        std::map<playerID_t, uint16_t> player_lookup; // playerID -> index in players
        std::map<int, Bullet> bullets;

        std::vector<int> deadBullets;

        float playerSize = 15.0f;
        float playerDonut = 6.0f;

        float colorTolerance = 0.1f;

        int numBots = 100;

        int numPlayers = 0;
        int numBullets = 0;

        bool finished = false;

        unsigned int gameBeginTime = 0;

        unsigned int lastUpdateTime = 0;

        unsigned int roundLength = 1000 * 60 * 10;

        float deltaTime = 0.0f;

        Game(const GameStart & gs) : numBots(gs.numBots), playerSize(gs.playerSize),
            playerDonut(gs.playerDonut), mapSize(gs.size)
        {}

        ~Game() {}

        void begin();

        void update();

        bool bulletCollide(const Player & p, const Bullet & b);

        bool constrainPos(glm::vec2 & pos, const float & radius)
        {
            bool changed = false;
            for(int i = 0; i < 2; i++)
            {
                auto next = glm::clamp(pos[i], radius, mapSize[i] - radius);
                if (next != pos[i])
                {
                    changed = true;
                    pos[i] = next;
                }
            }
            return changed;
        }

        void addBot(int colorId);

        Player * addHuman();
        Player * addHuman(glm::vec4 color);

        void addPlayer(int playerId, bool human, int colorId);

        unsigned int requestBullet(const Player * p);
};
