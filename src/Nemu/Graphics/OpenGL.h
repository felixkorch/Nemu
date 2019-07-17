#pragma once
#ifdef NEMU_PLATFORM_WEB
#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#define GLFW_INCLUDE_NONE
#define GLFW_USE_ES3
#include "GLFW/glfw3.h"
#include "GLES3/gl3.h"
#else
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#endif