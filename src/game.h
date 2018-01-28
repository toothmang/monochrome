#pragma once

#include "player.h"

#include "glm/vec2.hpp"
#include <vector>

class Game
{
    public:
        glm::vec2 mapSize;
        std::vector<Player> players;
        std::vector<glm::vec4> colors;

        float colorTolerance = 0.1f;

        int numBots = 10;

        int numPlayers = 0;

        bool finished = false;

        unsigned int gameBeginTime = 0;

        unsigned int lastUpdateTime = 0;

        float deltaTime = 0.0f;

        Game(glm::vec2 s) : mapSize(s) {}

        ~Game() {}

        void begin();

        void update();

        void constrainPos(glm::vec2 & pos, const float & radius)
        {
            for(int i = 0; i < 2; i++)
            {
                pos[i] = glm::clamp(pos[i], 0.0f, mapSize[i]);
            }
        }

        void addBot(int colorId);

        Player * addHuman();
        Player * addHuman(glm::vec4 color);

        void addPlayer(int playerId, bool human, int colorId);
};

glm::vec3 hsv2rgb(float hue, float sat, float val);
