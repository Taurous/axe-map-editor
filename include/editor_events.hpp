#pragma once

#include <allegro5/allegro.h>
#include "map.hpp"

enum
{
    AXE_EDITOR_EVENT_MOVE_VIEW = ALLEGRO_GET_EVENT_TYPE('A','X','E','E'),
    AXE_EDITOR_EVENT_ZOOM_IN,
    AXE_EDITOR_EVENT_ZOOM_OUT,
    AXE_EDITOR_EVENT_SHOWHIDE_GRID,
    AXE_EDITOR_EVENT_COPY_DATA
};