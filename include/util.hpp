#pragma once

#include <allegro5/allegro.h>
#include "vec.hpp"

vec2i getScreenSize();
std::string getAllegroVersionStr();
ALLEGRO_DISPLAY *createDisplay(std::string title, int width, int height, int flags);