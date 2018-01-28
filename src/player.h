#pragma once

#include "glm/glm.hpp"

class PlayerInput
{
    public:
        glm::vec2 move = glm::vec2(0.0f, 0.0f);
        float heading = 0.0f;
        bool firing{false};
};

class Player
{
    public:
        unsigned int lastUpdate;
        unsigned int botMin = 1000, botMax = 2000;
        unsigned int botWait = 0;
        bool isHuman = false;
        int id = 0;
        int colorId = 0;
        glm::vec2 pos = {0.0f, 0.0f};
        float heading = 0.0f;
        float size = 30.0f;
        glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

        PlayerInput input;

        Player(int _id) : id(_id) { }
        Player(int _id, bool human, glm::vec4 col)
            : this(_id), isHuman(human), color(col)
        {

        }

        virtual ~Player() {}

        void update();
};