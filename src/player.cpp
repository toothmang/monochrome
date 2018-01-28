#include "player.h"
#include "game.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"

#include "monochrome_client.h"

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


            input.move = glm::vec2(lx, ly);
            input.heading = glm::atan(rx, ry);
        }    
    }
    else
    {
        auto currentTime = SDL_GetTicks();
        auto diff = currentTime - lastUpdate;

        if (diff > botWait)
        {
            static auto pi = glm::pi<float>();
            input.move = glm::diskRand(1.0f);
            input.heading = glm::linearRand(-pi, pi);

            botWait = glm::linearRand(botMin, botMax);
            lastUpdate = currentTime;
        }
    }

    if (input.move.length() < 0.1f)
    {
        input.move = {0.0f, 0.0f};
    }

    vel = input.move * maxSpeed * game->deltaTime;
    pos = pos + vel;
    heading = input.heading;

    // if (isHuman)
    // {
    //     printf("input move: %.2f, %.2f\tvelocity: %.2f, %.2f\tpos: %.2f, %.2f, heading: %.2f\n", 
    //         input.move.x, input.move.y,
    //         vel.x, vel.y,
    //         pos.x, pos.y,
    //         input.heading);
    // }

    
}