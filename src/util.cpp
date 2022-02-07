#include <allegro5/allegro.h>
#include "util.hpp"

vec2i getScreenSize()
{
	return vec2i{al_get_display_width(al_get_current_display()), al_get_display_height(al_get_current_display())};
}

void printAllegroVersion()
{
	uint32_t version = al_get_allegro_version();
	int major = version >> 24;
	int minor = (version >> 16) & 255;
	int revision = (version >> 8) & 255;
	int release = version & 255;

	printf("Allegro version %i.%i.%i[%i]\n", major, minor, revision, release);
}