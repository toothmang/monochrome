#pragma once

#include "glm/glm.hpp"
#include <vector>


class CircleRenderer
{
    public:
    static CircleRenderer & get()
    {
        static CircleRenderer instance;
        return instance;
    }

    static int roundness;

    unsigned int vboHandle = 0;
    unsigned int elementsHandle = 0;
    int shaderHandle = 0, vertexHandle = 0, fragHandle = 0;
    int uniformMVPLoc = 0, uniformCenterLoc = 0, uniformRadiusLoc = 0, uniformMinRadiusLoc = 0,
        uniformColorLoc = 0, uniformMapSizeLoc = 0, uniformTexLoc = 0, uniformTexScaleLoc = 0;
    int attribPosLoc = 0, attribUVLoc = 0;

    bool initialized = false;

    std::vector<glm::vec4> vertices;
    std::vector<unsigned int> indices;

    virtual ~CircleRenderer();

    void shutdown();

    bool init();

    void render(const glm::vec2 & p, const glm::vec4 & color, float radius, float minRadius = 0.0f, bool useTex=false);
};
