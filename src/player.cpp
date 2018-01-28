#include "player.h"
#include "game.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"

#include "monochrome_client.h"

float Player::maxSpeed = 100.0f;
unsigned int Player::fireRate = 1000;

void Player::update()
{
    lastInput = input;

    if (isHuman)
    {
        auto & client = monochrome_client::get();

        auto & l = client.input.keyboardState[SDLK_a];
        auto & r = client.input.keyboardState[SDLK_d];
        auto & u = client.input.keyboardState[SDLK_w];
        auto & d = client.input.keyboardState[SDLK_s];

        glm::vec2 left = {0.0f, 0.0f}, right = {0.0f, 0.0f};

        left.x = l ? -1.0f : r ? 1.0f : 0.0f;
        left.y = d ? 1.0f : u ? -1.0f : 0.0f;

        auto mouseDiff = client.input.mousePos - pos;
        right = glm::normalize(mouseDiff);

        auto & lmb = client.input.mouseState[SDL_BUTTON_LEFT];

        input.firing = lmb == SDL_PRESSED;

        // Allow for post-init controller detection. 
        if (!client.controller)
        {
            int numJoysticks = SDL_NumJoysticks();

            //Check for joysticks
            if( numJoysticks >= 1 )
            {
                for(int i = 0; i < numJoysticks; i++)
                {
                    if (SDL_IsGameController(i))
                    {
                        printf("Using controller index %i, name %s\n", i, SDL_GameControllerNameForIndex(i));
                        client.controller = SDL_GameControllerOpen(i);
                        client.joystick = SDL_GameControllerGetJoystick(client.controller);
                        break;
                    }
                }
            }
        }

        // Overwrite with controller input if we aren't detecting anything
        if (glm::length(left) < 0.1f && client.controller)
        {
            left.x = glm::clamp(
                (float)SDL_GameControllerGetAxis(client.controller, SDL_CONTROLLER_AXIS_LEFTX) /  32767.0f,
                -1.0f, 1.0f);
            left.y = glm::clamp(
                (float)SDL_GameControllerGetAxis(client.controller, SDL_CONTROLLER_AXIS_LEFTY) /  32767.0f,
                -1.0f, 1.0f);

            if (!input.firing)
            {
                right.x = glm::clamp(
                    (float)SDL_GameControllerGetAxis(client.controller, SDL_CONTROLLER_AXIS_RIGHTX) /  32767.0f,
                    -1.0f, 1.0f);
                right.y = glm::clamp(
                    (float)SDL_GameControllerGetAxis(client.controller, SDL_CONTROLLER_AXIS_RIGHTY) /  32767.0f,
                    -1.0f, 1.0f);

                input.firing = glm::length(input.aim) > 0.2f;
            }
        }

        input.move = left;
        input.aim = right;
    }
    else
    {
        auto currentTime = game->lastUpdateTime;
        auto diff = currentTime - lastUpdate;

        if (diff > botWait)
        {
            lastInput = botInput;
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
                botInput.aim = minDir;
            }
            else
            {
                botInput.aim = glm::diskRand(1.0f);
            }

            // auto teamDiff = game->colorStats[colorId].avgPos - pos;
            // auto teamDist = glm::length(teamDiff);

            // if (teamDist > 10.0 * size)
            // {
            //     botInput.move = glm::normalize(teamDiff);
            // }
            // else
            {
                botInput.move = glm::diskRand(1.0f);
            }


            botInput.firing = glm::linearRand(0.0f, 1.0f) > 0.5f;

            botWait = glm::linearRand(botMin, botMax);
            lastUpdate = currentTime;
        }

        float t = (float)diff / botWait;

        input.move = glm::mix(lastInput.move, botInput.move, t);
        input.aim = glm::mix(lastInput.aim, botInput.aim, t);
        input.firing = botInput.firing;
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
