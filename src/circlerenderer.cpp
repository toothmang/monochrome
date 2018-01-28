#include "circlerenderer.h"
#include "glm/gtc/constants.hpp"
#include <SDL2/SDL_opengles2.h>

#include "monochrome_client.h"
#include "game.h"

#include <string>



int CircleRenderer::roundness = 4;

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
    #ifdef __EMSCRIPTEN__
        // WebGL requires precision specifiers but OpenGL 2.1 disallows
        // them, so I define the shader without it and then add it here.
        "precision mediump float;\n"
#endif
        "uniform mat4 MVP;\n"
        "uniform vec2 center;\n"
        "uniform float radius;\n"
        "attribute vec2 pos;\n"
        "attribute vec2 uv;\n"
        "varying vec2 v_uv;\n"
        "const float sqrt_2 = sqrt(2.0);\n"
        "void main()\n"
        "{\n"
        "   v_uv = uv;\n"
        "	gl_Position = MVP * vec4(center + (pos * radius * sqrt_2),0,1);\n"
        "}\n";

    const GLchar* fragment_shader =
#ifdef __EMSCRIPTEN__
        // WebGL requires precision specifiers but OpenGL 2.1 disallows
        // them, so I define the shader without it and then add it here.
        "precision mediump float;\n"
#endif
        "uniform vec4 color;\n"
        "uniform vec2 center;\n"
        "uniform vec2 mapSize;\n"
        "uniform float radius;\n"
        "varying vec2 v_uv;\n"
        "void main()\n"
        "{\n"
        "   float dist = length(gl_FragCoord.xy - vec2(center.x,mapSize.y-center.y));\n"
        "   if (dist > radius) discard;\n"
        "	gl_FragColor = color;\n"
        "}\n";

    shaderHandle = glCreateProgram();
    vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexHandle, 1, &vertex_shader, 0);
    glShaderSource(fragHandle, 1, &fragment_shader, 0);

    int didCompile = 0, logLength = 0;

    glCompileShader(vertexHandle);
	glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &didCompile);
    glGetShaderiv(vertexHandle, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
        std::vector<GLchar> shaderLog(logLength + 1);
		glGetShaderInfoLog(vertexHandle, logLength, &logLength, shaderLog.data());
        printf("Vertex shader log:\n%s\n", std::string(shaderLog.begin(), shaderLog.end()).c_str());
	}

    glCompileShader(fragHandle);
    glGetShaderiv(fragHandle, GL_COMPILE_STATUS, &didCompile);
    glGetShaderiv(fragHandle, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
        std::vector<GLchar> shaderLog(logLength + 1);
		glGetShaderInfoLog(fragHandle, logLength, &logLength, shaderLog.data());
        printf("Frag shader log:\n%s\n", std::string(shaderLog.begin(), shaderLog.end()).c_str());
	}
    glAttachShader(shaderHandle, vertexHandle);
    glAttachShader(shaderHandle, fragHandle);
    glLinkProgram(shaderHandle);

    GLint link_ok = GL_FALSE;
	glGetProgramiv(shaderHandle, GL_LINK_STATUS, &link_ok);

    if (!link_ok)
	{
		int logLength = 0;
		glGetProgramiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<GLchar> shaderLog(logLength + 1);
		glGetProgramInfoLog(shaderHandle, logLength, &logLength, shaderLog.data());
 
		std::string linkLog = std::string(shaderLog.begin(), shaderLog.end());

		printf("Shader did not link. Log:\%s\n", linkLog.c_str());
	}

    uniformMVPLoc = glGetUniformLocation(shaderHandle, "MVP");
    uniformCenterLoc = glGetUniformLocation(shaderHandle, "center");
    uniformRadiusLoc = glGetUniformLocation(shaderHandle, "radius");
    uniformColorLoc = glGetUniformLocation(shaderHandle, "color");
    uniformMapSizeLoc = glGetUniformLocation(shaderHandle, "mapSize");
    
    attribPosLoc = glGetAttribLocation(shaderHandle, "pos");
    attribUVLoc = glGetAttribLocation(shaderHandle, "uv");
    
    vertices.clear();
    indices.clear();

    vertices.push_back(glm::vec4(0.0f, 0.0f, 0.5f, 0.5f));

    float theta = glm::two_pi<float>() / roundness;

    static glm::vec2 uvs[] = 
    {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f)
    };

    for(int i = 0; i < roundness; i++)
    {
        float t = theta * i;

        vertices.push_back(glm::vec4(glm::cos(t), glm::sin(t), uvs[i].x, uvs[i].y));
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
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
    glUniform2fv(uniformMapSizeLoc, 1, &client.game->mapSize[0]);

    // Render command lists
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    glEnableVertexAttribArray(attribPosLoc);
    glEnableVertexAttribArray(attribUVLoc);
    glVertexAttribPointer(attribPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)0);
    glVertexAttribPointer(attribUVLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)sizeof(glm::vec2));
#undef OFFSETOF

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsHandle);

    glDrawElements(GL_TRIANGLE_FAN, indices.size(), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(attribPosLoc);
    glDisableVertexAttribArray(attribUVLoc);

    glUseProgram(0);
}