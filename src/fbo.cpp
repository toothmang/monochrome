
#include "fbo.hpp"
//#include "SDL2/SDL_opengl.h"
#include <SDL2/SDL_opengles2.h>

#include <stdio.h>

FBO::FBO(int texwidth, int texheight)
 : width(texwidth), height(texheight)
{
    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &colTexID);
    glBindTexture(GL_TEXTURE_2D, colTexID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbID);
    glBindFramebuffer(GL_FRAMEBUFFER, fbID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colTexID, 0);

    // glGenRenderbuffers(1, &depthBuffer);
    // glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    //glDrawBuffer(GL_COLOR_ATTACHMENT0);
    //glGenTextures(1, &depthTexID);
    //glBindTexture(GL_TEXTURE_2D, depthTexID);



    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

    //glDrawBuffer(GL_DEPTH_ATTACHMENT);


    unsigned int FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(FBOstatus != GL_FRAMEBUFFER_COMPLETE)
        printf("GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO. Error %d\n", FBOstatus);
    else
        printf("GL_FRAMEBUFFER_COMPLETE -> success!\n");

    //glDrawBuffers(2, {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT});
}

FBO::~FBO()
{
    glDeleteFramebuffers(1, &fbID);
}

void FBO::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbID);
    //glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    //glDisable(GL_DEPTH_TEST);
    //glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FBO::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glBindRenderbuffer(GL_RENDERBUFFER, 0);
    //glBindTexture(GL_TEXTURE_2D, 0);
}

void FBO::blank()
{
    bind();
    glClearColor(0., 0., 1., 0.);
    glClear( GL_COLOR_BUFFER_BIT );
    unbind();
}

void FBO::load()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, colTexID);
}
