#pragma once

#include <string>
#include <allegro5/allegro.h>

#include "vec.hpp"

struct Tilemap
{
	ALLEGRO_BITMAP* bmp = nullptr;
	std::string path;

	vec2f tile_size = { 32, 32 };
	vec2f size = { 1, 1 };

	int max_tiles = 1;
};

Tilemap* loadTilemap(std::string path, vec2i tile_size);
void destroyTilemap(Tilemap* t);