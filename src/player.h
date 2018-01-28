#pragma once

#include "glm/glm.hpp"
#include "messages.hpp"

class Game;

class PlayerInput
{
    public:
        glm::vec2 move = {0.0f, 0.0f};
        glm::vec2 aim = {0.0f, 0.0f};
        bool firing = false;
};

class Player
{
    public:
        Game * game = nullptr;

        bool isHuman = false;
        int id = 0;
        int colorId = 0;
        glm::vec2 pos = {0.0f, 0.0f};
        glm::vec2 vel = {0.0f, 0.0f};
        glm::vec2 headingPos = {0.0f, 0.0f};
        timestamp_t timestamp = 0;
        float size = 15.0f;
        float minSize = 0.0f;


        unsigned int lastUpdate = 0;
        unsigned int lastFireTime = 0;
        
        unsigned int botMin = 100, botMax = 500;
        unsigned int botWait = 0;

        static float maxSpeed;
        static unsigned int fireRate;

        PlayerInput input, lastInput, botInput;

        Player(Game * g, const glm::vec2 & p, int _id, bool human, int colId,
            float _size, float _minSize = 0.0f)
            : game(g), pos(p), id(_id), isHuman(human), colorId(colId),
            size(_size), minSize(_minSize), timestamp(0.)
        {

        }

        virtual ~Player() {}

        static glm::vec2 headingDir(float h)
        {
            return glm::vec2(glm::cos(h), glm::sin(h));
        }

        void update();
};
