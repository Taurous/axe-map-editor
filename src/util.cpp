#include "util.hpp"

vec2f getScreenSize()
{
	return vec2f(al_get_display_width(al_get_current_display()), al_get_display_height(al_get_current_display()));
}