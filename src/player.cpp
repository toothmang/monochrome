#include "player.h"

#include "glm/gtc/random.hpp"

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
            input.move = glm::diskRand(1.0f);
            input.heading = glm::gtx::random(-glm::pi(), glm::pi());

            botWait = glm::gtx::random(botMin, botMax);
            lastUpdate = currentTime;
        }
    }

    pos += input.move;
    heading = input.heading;
}