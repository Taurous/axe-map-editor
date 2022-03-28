#pragma once

#include "imgui.h"
#include "imgui_impl_allegro5.h"
#include "util.hpp"

#include <list>
#include <string>

#include <allegro5/allegro.h>

enum
{
    AXE_GUI_EVENT_ADD_CREATURE = ALLEGRO_GET_EVENT_TYPE('G','A','X','E'),
    AXE_GUI_EVENT_QUIT
};

enum
{
    SIDE_WIDTH = 300,
    BOTTOM_BAR_HEIGHT = 40
};

ImGuiIO& initGui(ALLEGRO_DISPLAY* d);
void renderGui(std::list<std::string>& creatures, ALLEGRO_EVENT_SOURCE *src);