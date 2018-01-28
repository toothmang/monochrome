#pragma once

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

class GameStart
{
    public:
        int numBots = 64;
        float playerSize = 15.0f;
        float playerDonut = 9.0f;
        bool usePlayerColor = false;
        glm::vec4 playerColor = glm::vec4(1.0f);
        glm::vec2 size;
};