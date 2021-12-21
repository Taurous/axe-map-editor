#include "tilemap.hpp"

#include <iostream>

Tilemap* loadTilemap(std::string path, vec2i tile_size)
{
	Tilemap* t = new Tilemap;

	t->bmp = nullptr;
	t->bmp = al_load_bitmap(path.c_str());
	if (t->bmp == nullptr)
	{
		std::cerr << "Failed to load tilemap: " << path << std::endl;
		delete t;
		return nullptr;
	}

	t->tile_size = tile_size;
	t->size.x = (float)(al_get_bitmap_width(t->bmp) / tile_size.x);
	t->size.y = (float)(al_get_bitmap_height(t->bmp) / tile_size.y);

	t->max_tiles = t->size.x * t->size.y;

	t->path = path;

	return t;
}

void destroyTilemap(Tilemap* t)
{
	if (t)
	{
		if (t->bmp)
		{
			al_destroy_bitmap(t->bmp);
		}
		delete t;
	}
}