#pragma once

#include "glm/glm.hpp"

class Game;

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
        Game * game = nullptr;
        unsigned int lastUpdate;
        unsigned int botMin = 1000, botMax = 2000;
        unsigned int botWait = 0;
        bool isHuman = false;
        int id = 0;
        int colorId = 0;
        glm::vec2 pos = {0.0f, 0.0f};
        glm::vec2 vel = {0.0f, 0.0f};
        float heading = 0.0f;
        float size = 30.0f;
        float maxSpeed = 100.0f;
        glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

        PlayerInput input;

        Player(Game * g, int _id, bool human, int colId)
            : game(g), id(_id), isHuman(human), colorId(colId)
        {

        }

        virtual ~Player() {}

        void update();
};