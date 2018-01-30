
#include "fbo.hpp"
//#include "SDL2/SDL_opengl.h"
#include <SDL2/SDL_opengles2.h>

#include "glm/gtc/constants.hpp"
#include <stdio.h>
#include <string>



bool FSQuad::init()
{
    const GLchar *vertex_shader =

        "precision mediump float;\n"
        "attribute vec4 pos;\n"
        "varying vec2 v_uv;\n"
        "void main()\n"
        "{\n"
        "   v_uv = pos.zw;\n"
        "	gl_Position.xy = pos.xy;\n"
        "   gl_Position.zw = vec2(-0.1, 1.0);\n"
        "}\n";

    const GLchar* fragment_shader =

        "precision mediump float;\n"
        "uniform sampler2D tex;\n"
        "uniform vec4 color;\n"
        "varying vec2 v_uv;\n"
        "void main()\n"
        "{\n"
        "	gl_FragColor.rgb = texture2D(tex, v_uv.xy).rgb * color.rgb;\n"
        "   gl_FragColor.a = 1.;\n"
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

    if (!didCompile)
    {
        printf("Vertex shader failed to compile\n");
        return false;
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
    if (!didCompile)
    {
        printf("Frag shader failed to compile\n");
        return false;
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

    uniformTexLoc = glGetUniformLocation(shaderHandle, "tex");
    uniformColorLoc = glGetUniformLocation(shaderHandle, "color");

    attribPosLoc = glGetAttribLocation(shaderHandle, "pos");
    //attribUVLoc = glGetAttribLocation(shaderHandle, "uv");

    indices.clear();
    xy_uv.clear();

    // vertices.push_back(glm::vec2(-1.,-1.)); texcoords.push_back(glm::vec2(0.,0.));
    // vertices.push_back(glm::vec2( 1.,-1.)); texcoords.push_back(glm::vec2(1.,0.));
    // vertices.push_back(glm::vec2( 1., 1.)); texcoords.push_back(glm::vec2(1.,1.));
    // vertices.push_back(glm::vec2(-1., 1.)); texcoords.push_back(glm::vec2(0.,1.));

    xy_uv.push_back(glm::vec4(-1.,-1., 0.,0.));
    xy_uv.push_back(glm::vec4( 1.,-1., 1.,0.));
    xy_uv.push_back(glm::vec4( 1., 1., 1.,1.));
    xy_uv.push_back(glm::vec4(-1., 1., 0.,1.));

    // First triangle: 0 1 3
    indices.push_back(0); indices.push_back(1); indices.push_back(3);
    // Second triangle 3 1 2
    indices.push_back(3); indices.push_back(1); indices.push_back(2);

    glGenBuffers(1, &vboHandle);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * xy_uv.size(), &xy_uv[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &elementsHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

FSQuad::~FSQuad()
{
    shutdown();
}

void FSQuad::shutdown()
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

void FSQuad::render(glm::vec4 color = glm::vec4(1.))
{
    if (!initialized)
    {
        initialized = init();
        if (!initialized)
        {
            return;
        }
    }

    //auto & client = monochrome_client::get();

    glUseProgram(shaderHandle);

    glUniform4fv(uniformColorLoc, 1, &color[0]);
    glUniform1i(uniformTexLoc, 0);

    // Render command lists
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    glEnableVertexAttribArray(attribPosLoc);
    glVertexAttribPointer(attribPosLoc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsHandle);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(attribPosLoc);
    //glDisableVertexAttribArray(attribUVLoc);
    glUseProgram(0);
}



FBO::FBO(int texwidth, int texheight)
 : width(texwidth), height(texheight)
{
    glGenTextures(1, &colTexID);
    glBindTexture(GL_TEXTURE_2D, colTexID);
    // WARNING: FBO texture dimensions MUST match framebuffer dimensions
    //          Non power-of-2 textures MUST use GL_CLAMP_TO_EDGE
    //    GL_REPEAT or anything else here will set no error flags, but will SILENTLY FAIL
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbID);
    glBindFramebuffer(GL_FRAMEBUFFER, fbID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colTexID, 0);

    // Untested code -- We don't need the depth renderbuffer, because we don't use depth tests!
    // glGenRenderbuffers(1, &depthBuffer);
    // glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    // glGenTextures(1, &depthTexID);
    // glBindTexture(GL_TEXTURE_2D, depthTexID);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    //  Untested //

    unsigned int FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(FBOstatus != GL_FRAMEBUFFER_COMPLETE)
        printf("GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO. Error %d\n", FBOstatus);
    else
        printf("GL_FRAMEBUFFER_COMPLETE -> success!\n");
}

FBO::~FBO()
{
    glDeleteFramebuffers(1, &fbID);
}

void FBO::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbID);
    //glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);

    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FBO::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void FBO::blank()
{
    bind();
    glClearColor(0., 0., 0., 1.);
    glClear( GL_COLOR_BUFFER_BIT );
    unbind();
}

void FBO::load()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, colTexID);
}
