#include "circlerenderer.h"
#include "glm/gtc/constants.hpp"
#include <SDL2/SDL_opengles2.h>

#include "monochrome_client.h"



int CircleRenderer::roundness = 16;

CircleRenderer::~CircleRenderer()
{
    shutdown();
}

void CircleRenderer::shutdown()
{
    if (vboHandle)
    {
        glDeleteBuffers(1, &vboHandle);
        vboHandle = 0;
    }
    if (elementsHandle)
    {
        glDeleteBuffers(1, &elementsHandle);
        elementsHandle = 0;
    }

    glDetachShader(shaderHandle, vertexHandle);
    glDeleteShader(vertexHandle);
    vertexHandle = 0;

    glDetachShader(shaderHandle, fragHandle);
    glDeleteShader(fragHandle);
    fragHandle = 0;

    glDeleteProgram(shaderHandle);
    shaderHandle = 0;
}

bool CircleRenderer::init()
{
    const GLchar *vertex_shader =
        "uniform mat4 MVP;\n"
        "uniform vec2 center;\n"
        "uniform float radius;\n"
        "attribute vec2 pos;\n"
        "void main()\n"
        "{\n"
        "	gl_Position = MVP * vec4(center + (pos * radius),0,1);\n"
        "}\n";

    const GLchar* fragment_shader =
#ifdef __EMSCRIPTEN__
        // WebGL requires precision specifiers but OpenGL 2.1 disallows
        // them, so I define the shader without it and then add it here.
        "precision mediump float;\n"
#endif
        "uniform vec4 color;\n"
        "void main()\n"
        "{\n"
        "	gl_FragColor = color;\n"
        "}\n";

    shaderHandle = glCreateProgram();
    vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexHandle, 1, &vertex_shader, 0);
    glShaderSource(fragHandle, 1, &fragment_shader, 0);
    glCompileShader(vertexHandle);
    glCompileShader(fragHandle);
    glAttachShader(shaderHandle, vertexHandle);
    glAttachShader(shaderHandle, fragHandle);
    glLinkProgram(shaderHandle);

    uniformMVPLoc = glGetUniformLocation(shaderHandle, "MVP");
    uniformCenterLoc = glGetUniformLocation(shaderHandle, "center");
    uniformRadiusLoc = glGetUniformLocation(shaderHandle, "radius");
    uniformColorLoc = glGetUniformLocation(shaderHandle, "color");
    
    attribPosLoc = glGetAttribLocation(shaderHandle, "pos");
    
    vertices.clear();
    indices.clear();

    vertices.push_back(glm::vec2(0.0f));

    float theta = glm::two_pi<float>() / roundness;

    for(int i = 0; i < roundness; i++)
    {
        float t = theta * i;

        vertices.push_back(glm::vec2(glm::cos(t), glm::sin(t)));
        //indices.push_back(0);
        indices.push_back(i + 1);

        if (i == roundness - 1)
        {
            indices.push_back(1);
        }
        else
        {
            indices.push_back(i + 2);
        }
        
    }

    glGenBuffers(1, &vboHandle);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &elementsHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

void CircleRenderer::render(const glm::vec2 & p, float radius, const glm::vec4 & color)
{
    if (!initialized)
    {
        initialized = init();
        if (!initialized)
        {
            return;
        }
    }

    auto & client = monochrome_client::get();

    // Setup orthographic projection matrix
    const float mvp[4][4] =
    {
        { 2.0f/client.width, 0.0f,                   0.0f, 0.0f },
        { 0.0f,                  2.0f/-client.height, 0.0f, 0.0f },
        { 0.0f,                  0.0f,                  -1.0f, 0.0f },
        {-1.0f,                  1.0f,                   0.0f, 1.0f },
    };

    glUseProgram(shaderHandle);
    
    glUniformMatrix4fv(uniformMVPLoc, 1, GL_FALSE, &mvp[0][0]);
    glUniform2fv(uniformCenterLoc, 1, &p[0]);
    glUniform1f(uniformRadiusLoc, radius);
    glUniform4fv(uniformColorLoc, 1, &color[0]);

    // Render command lists
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    glEnableVertexAttribArray(attribPosLoc);
    glVertexAttribPointer(attribPosLoc, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsHandle);

    glDrawElements(GL_TRIANGLE_FAN, indices.size(), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(attribPosLoc);

    glUseProgram(0);
}