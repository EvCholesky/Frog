#pragma once

#include "glfw/include/GLFW/glfw3.h"
#include "glfw/deps/GL/glext.h"

//GLSL extensions

#define FR_GL_EXTENSIONS \
FR_FUNCTION_TYPE(PFNGLACTIVETEXTUREARBPROC)			FR_FUNCTION_NAME(glActiveTextureARB)		\
FR_FUNCTION_TYPE(PFNGLATTACHSHADERPROC) 			FR_FUNCTION_NAME(glAttachShader) 			\
FR_FUNCTION_TYPE(PFNGLCOMPILESHADERPROC)			FR_FUNCTION_NAME(glCompileShader) 			\
FR_FUNCTION_TYPE(PFNGLCLIENTACTIVETEXTUREARBPROC)	FR_FUNCTION_NAME(glClientActiveTextureARB)	\
FR_FUNCTION_TYPE(PFNGLCREATEPROGRAMPROC)			FR_FUNCTION_NAME(glCreateProgram) 			\
FR_FUNCTION_TYPE(PFNGLCREATESHADERPROC)				FR_FUNCTION_NAME(glCreateShader) 			\
FR_FUNCTION_TYPE(PFNGLDELETESHADERPROC) 			FR_FUNCTION_NAME(glDeleteShader) 			\
FR_FUNCTION_TYPE(PFNGLDELETEPROGRAMPROC) 			FR_FUNCTION_NAME(glDeleteProgram) 			\
FR_FUNCTION_TYPE(PFNGLGETSHADERIVPROC)				FR_FUNCTION_NAME(glGetShaderiv) 			\
FR_FUNCTION_TYPE(PFNGLGETINFOLOGARBPROC)			FR_FUNCTION_NAME(glGetInfoLogARB) 			\
FR_FUNCTION_TYPE(PFNGLGETPROGRAMINFOLOGPROC)		FR_FUNCTION_NAME(glGetProgramInfoLog)		\
FR_FUNCTION_TYPE(PFNGLGETPROGRAMIVPROC)				FR_FUNCTION_NAME(glGetProgramiv)			\
FR_FUNCTION_TYPE(PFNGLGETSHADERINFOLOGPROC)			FR_FUNCTION_NAME(glGetShaderInfoLog) 		\
FR_FUNCTION_TYPE(PFNGLGETUNIFORMLOCATIONPROC)		FR_FUNCTION_NAME(glGetUniformLocation) 		\
FR_FUNCTION_TYPE(PFNGLLINKPROGRAMPROC)				FR_FUNCTION_NAME(glLinkProgram) 			\
FR_FUNCTION_TYPE(PFNGLSHADERSOURCEPROC) 			FR_FUNCTION_NAME(glShaderSource) 			\
FR_FUNCTION_TYPE(PFNGLUNIFORM4FARBPROC)				FR_FUNCTION_NAME(glUniform4fARB) 			\
FR_FUNCTION_TYPE(PFNGLUNIFORMMATRIX4FVARBPROC)		FR_FUNCTION_NAME(glUniformMatrix4fvARB) 	\
FR_FUNCTION_TYPE(PFNGLUNIFORM1IARBPROC)				FR_FUNCTION_NAME(glUniform1iARB) 			\
FR_FUNCTION_TYPE(PFNGLUSEPROGRAMPROC)				FR_FUNCTION_NAME(glUseProgram)			 	\
FR_FUNCTION_TYPE(PFNGLBINDATTRIBLOCATIONPROC)		FR_FUNCTION_NAME(glBindAttribLocation)		\
FR_FUNCTION_TYPE(PFNGLCLIENTACTIVETEXTUREPROC)		FR_FUNCTION_NAME(glClientActiveTexture)

#define FR_FUNCTION_TYPE(TYPE_NAME) extern TYPE_NAME
#define FR_FUNCTION_NAME(FUNC_NAME) FUNC_NAME;
FR_GL_EXTENSIONS
#undef FR_FUNCTION_TYPE
#undef FR_FUNCTION_NAME

#ifdef GL_WRAPPER_IMPLEMENTATION
#ifndef GL_WRAPPER_IMPLEMENTATION_GUARD
#define GL_WRAPPER_IMPLEMENTATION_GUARD

#include "Common.h"
#include <stdio.h>

#define FR_FUNCTION_TYPE(TYPE_NAME) TYPE_NAME
#define FR_FUNCTION_NAME(FUNC_NAME) FUNC_NAME=0;
FR_GL_EXTENSIONS
#undef FR_FUNCTION_TYPE
#undef FR_FUNCTION_NAME

bool Frog_FTryStaticInitGLExtensions()
{
	int cError = 0;

	#define FR_FUNCTION_TYPE(TYPE_NAME) { TYPE_NAME ptr = (TYPE_NAME)glfwGetProcAddress(
	#define FR_FUNCTION_NAME(FUNC_NAME) 	#FUNC_NAME); \
										FUNC_NAME = ptr; \
										if(FUNC_NAME == 0) \
										{ \
											printf("Error %d, unable to bind GL extension: %s\n", cError++, #FUNC_NAME); \
											return false; \
										} }
	FR_GL_EXTENSIONS
	#undef FR_FUNCTION_TYPE
	#undef FR_FUNCTION_NAME
	
	FR_ASSERT(!cError, "Failed to init GL extensions");
	return true;
}

#endif //GL_WRAPPER_IMPLEMENTATION_GUARD
#endif //GL_WRAPPER_IMPLEMENTATION
