#include "player.h"
#include "game.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"

#include "monochrome_client.h"

float Player::maxSpeed = 100.0f;
unsigned int Player::fireRate = 1000;

void Player::update()
{
    if (isHuman)
    {
        auto & client = monochrome_client::get();

        if (client.controller)
        {
            float lx = glm::clamp(
                (float)SDL_GameControllerGetAxis(client.controller, SDL_CONTROLLER_AXIS_LEFTX) /  32767, 
                -1.0f, 1.0f);
            float ly = glm::clamp(
                (float)SDL_GameControllerGetAxis(client.controller, SDL_CONTROLLER_AXIS_LEFTY) /  32767, 
                -1.0f, 1.0f);

            float rx = glm::clamp(
                (float)SDL_GameControllerGetAxis(client.controller, SDL_CONTROLLER_AXIS_RIGHTX) /  32767, 
                -1.0f, 1.0f);
            float ry = glm::clamp(
                (float)SDL_GameControllerGetAxis(client.controller, SDL_CONTROLLER_AXIS_RIGHTY) /  32767, 
                -1.0f, 1.0f);

            input.move = {lx, ly};
            input.aim = {rx, ry};
            input.firing = glm::length(input.aim) > 0.2f;
        }

    }
    else
    {
        auto currentTime = SDL_GetTicks();
        auto diff = currentTime - lastUpdate;

        if (diff > botWait)
        {
            // Aim at nearest non-color player
            glm::vec2 minDir = {0.0f, 0.0f};
            float minDist = FLT_MAX;

            for(auto & p : game->players)
            {
                if (p.id == id || p.colorId == colorId) continue;
                auto dir = p.pos - pos;
                auto d = glm::length(dir);
                if (d < minDist)
                {
                    minDist = d;
                    minDir = glm::normalize(dir);
                }
            }

            if (glm::length(minDir) > 0.0f)
            {
                input.aim = minDir;
            }
            else
            {
                input.aim = glm::diskRand(1.0f);
            }

            input.move = glm::diskRand(1.0f);
            input.firing = glm::linearRand(0.0f, 1.0f) > 0.5f;
            

            botWait = glm::linearRand(botMin, botMax);
            lastUpdate = currentTime;
        }
    }

    if (glm::length(input.move) < 0.2f)
    {
        input.move = {0.0f, 0.0f};
    }

    vel = input.move * maxSpeed * game->deltaTime;
    pos = pos + vel;
    headingPos = pos + input.aim * size;

    if (input.firing)
    {
        auto ft = game->requestBullet(this);

        if (ft)
        {
            lastFireTime = ft;
        }
    }

    // if (isHuman)
    // {
    //     printf("input move: %.2f, %.2f\tvelocity: %.2f, %.2f\tpos: %.2f, %.2f, heading: %.2f\n", 
    //         input.move.x, input.move.y,
    //         vel.x, vel.y,
    //         pos.x, pos.y,
    //         input.heading);
    // }

    
}