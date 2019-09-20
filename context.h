#pragma once
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include <stdint.h>
#include <experimental/filesystem>

#include "types.h"
#include "hash.h"

#include "curl/include/curl/curl.h"
#include "json/single_include/nlohmann/json.hpp"

#include "api.h"

#include <GL/gl3w.h>
#include <GL/GL.h>
#include <GL/GLU.h>
#include <SDL.h>
#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_sdl.h"
#include "imgui/examples/imgui_impl_opengl3.h"

// very funny
#undef main

class c_context
{
public:
};
inline c_context ctx;

#include "ui.h"