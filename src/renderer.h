#pragma once

#include "glm/glm.hpp"

class CircleRenderer
{
    public:
    static CircleRenderer & get()
    {
        static CircleRenderer instance;
        return instance;
    }

    int shaderHandle = 0, vertexHandle = 0, fragHandle = 0;
    int uniformMVPLoc = 0;
    int attribPosLoc = 0, attribRadiusLoc = 0;

    void render(const glm::vec2 & p, float radius);
}