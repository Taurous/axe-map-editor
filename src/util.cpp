#include <allegro5/allegro.h>
#include "util.hpp"

vec2i getScreenSize()
{
	return vec2i{al_get_display_width(al_get_current_display()), al_get_display_height(al_get_current_display())};
}