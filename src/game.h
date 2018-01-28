#pragma once

#include "player.h"

#include "glm/vec2.hpp"
#include <vector>

class Game
{
    public:
        glm::vec2 size;
        std::vector<Player> players;
        std::vector<glm::vec4> colors;

        float colorTolerance = 0.1f;

        int numBots = 10;

        int numPlayers = 0;

        bool finished = false;

        uint gameBeginTime = 0;

        Game(glm::vec2 s) : size(s) {}

        ~Game() {}

        void begin();
        
        void update();

        void constrainPos(glm::vec2 & pos, const float & radius)
        {
            for(int i = 0; i < 2; i++)
            {
                pos[i] = glm::clamp(pos[i], 0.0f, size[i]);
            }
        }

        void addBot(int colorId);

        Player * addPlayer(glm::vec4 color);
};