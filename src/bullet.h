#pragma once

#include "glm/glm.hpp"

class Bullet
{
    public:
        glm::vec2 pos = {0.0f, 0.0f};
        glm::vec2 vel = {0.0f, 0.0f};
        int id = 0;
        int playerId = 0;
        int colorId = 0;
        float size = 10.0f;
        unsigned int spawnTime = 0;

        Bullet() {}
        Bullet(const glm::vec2 & p, const glm::vec2 & v, int pid, int cid, float sz, unsigned int t)
            : pos(p), vel(v), playerId(pid), colorId(cid), size(sz), spawnTime(t)
        {
            
        }

        bool operator<(const Bullet & b)
        {
            return id < b.id;
        }

        void update(float dt)
        {
            pos += vel * dt;
        }
};