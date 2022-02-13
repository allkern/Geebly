#pragma once

#include "SDL_opengl.h"
#include "SDL_opengl_glext.h"

namespace shaders {
    PFNGLCREATESHADERPROC            glCreateShader;
    PFNGLSHADERSOURCEPROC            glShaderSource;
    PFNGLCOMPILESHADERPROC           glCompileShader;
    PFNGLGETSHADERIVPROC             glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
    PFNGLDELETESHADERPROC            glDeleteShader;
    PFNGLATTACHSHADERPROC            glAttachShader;
    PFNGLCREATEPROGRAMPROC           glCreateProgram;
    PFNGLLINKPROGRAMPROC             glLinkProgram;
    PFNGLVALIDATEPROGRAMPROC         glValidateProgram;
    PFNGLGETPROGRAMIVPROC            glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
    PFNGLUSEPROGRAMPROC              glUseProgram;
    PFNGLBINDFRAGDATALOCATIONPROC    glBindFragDataLocation;
    PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
    PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
    PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
    PFNGLGENBUFFERSPROC              glGenBuffers;
    PFNGLBINDBUFFERPROC              glBindBuffer;
    PFNGLBUFFERDATAPROC              glBufferData;
    PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLACTIVETEXTUREPROC           glActiveTexture1;
    PFNGLGENFRAMEBUFFERSPROC         glGenFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC         glBindFramebuffer;
    PFNGLFRAMEBUFFERTEXTUREPROC      glFramebufferTexture;
    PFNGLFRAMEBUFFERTEXTURE2DPROC    glFramebufferTexture2D;
    PFNGLDRAWBUFFERSPROC             glDrawBuffers;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC  glCheckFramebufferStatus;
    PFNGLGETTEXTUREIMAGEEXTPROC      glGetTextureImage;
    PFNGLUNIFORM1FPROC               glUniform1f;
    PFNGLUNIFORM2FPROC               glUniform2f;
    PFNGLUNIFORM3FPROC               glUniform3f;
    PFNGLUNIFORM4FPROC               glUniform4f;
    PFNGLUNIFORM1IPROC               glUniform1i;
    PFNGLUNIFORM2IPROC               glUniform2i;
    PFNGLUNIFORM3IPROC               glUniform3i;
    PFNGLUNIFORM4IPROC               glUniform4i;
    PFNGLUNIFORM1UIPROC              glUniform1ui;
    PFNGLUNIFORM2UIPROC              glUniform2ui;
    PFNGLUNIFORM3UIPROC              glUniform3ui;
    PFNGLUNIFORM4UIPROC              glUniform4ui;

    bool gl_functions_loaded = false;

    void load_gl_functions() {
        glCreateShader            = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
        glShaderSource            = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
        glCompileShader           = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
        glGetShaderiv             = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
        glGetShaderInfoLog        = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
        glDeleteShader            = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
        glAttachShader            = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
        glCreateProgram           = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
        glLinkProgram             = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
        glValidateProgram         = (PFNGLVALIDATEPROGRAMPROC)SDL_GL_GetProcAddress("glValidateProgram");
        glGetProgramiv            = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
        glGetProgramInfoLog       = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
        glUseProgram              = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
        glBindFragDataLocation    = (PFNGLBINDFRAGDATALOCATIONPROC)SDL_GL_GetProcAddress("glBindFragDataLocation");
        glGetUniformLocation      = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
        glGenVertexArrays         = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
        glBindVertexArray         = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
        glGenBuffers              = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
        glBindBuffer              = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
        glBufferData              = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
        glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
        glActiveTexture1          = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");
        glGenFramebuffers         = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
        glBindFramebuffer         = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
        glFramebufferTexture      = (PFNGLFRAMEBUFFERTEXTUREPROC)SDL_GL_GetProcAddress("glFramebufferTexture");
        glFramebufferTexture2D    = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
        glDrawBuffers             = (PFNGLDRAWBUFFERSPROC)SDL_GL_GetProcAddress("glDrawBuffers");
        glCheckFramebufferStatus  = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
        glUniform1f               = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f");
        glUniform2f               = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");
        glUniform3f               = (PFNGLUNIFORM3FPROC)SDL_GL_GetProcAddress("glUniform3f");
        glUniform4f               = (PFNGLUNIFORM4FPROC)SDL_GL_GetProcAddress("glUniform4f");
        glUniform1i               = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
        glUniform2i               = (PFNGLUNIFORM2IPROC)SDL_GL_GetProcAddress("glUniform2i");
        glUniform3i               = (PFNGLUNIFORM3IPROC)SDL_GL_GetProcAddress("glUniform3i");
        glUniform4i               = (PFNGLUNIFORM4IPROC)SDL_GL_GetProcAddress("glUniform4i");
        glUniform1ui              = (PFNGLUNIFORM1UIPROC)SDL_GL_GetProcAddress("glUniform1ui");
        glUniform2ui              = (PFNGLUNIFORM2UIPROC)SDL_GL_GetProcAddress("glUniform2ui");
        glUniform3ui              = (PFNGLUNIFORM3UIPROC)SDL_GL_GetProcAddress("glUniform3ui");
        glUniform4ui              = (PFNGLUNIFORM4UIPROC)SDL_GL_GetProcAddress("glUniform4ui");

        gl_functions_loaded = true;
    }
}