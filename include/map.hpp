#pragma once

#include "view.hpp"

#include <vector>

struct Map
{
	ALLEGRO_BITMAP* bmp = nullptr;
	std::string path;
	int width;
	int height;
	int tile_size;
	bool needs_save;

	std::vector<bool> v_tiles;
};

bool createMap(Map& m, std::string path_to_map, int tile_size);
void clearMap(Map& m);
bool reloadMap(Map& m);

bool saveMap(Map& m, std::string file, const View::ViewPort& v);
bool loadMap(Map& m, std::string file, View::ViewPort& v, bool restore_view);

void drawMap(const Map& m, const View::ViewPort& v, bool draw_grid, bool show_hidden);

void hideTile(Map& m, const vec2i& position);
void showTile(Map& m, const vec2i& position);
void setTile(Map& m, const vec2i& position, bool show);
bool isTileShown(const Map& m, const vec2i& position);

vec2i getTilePos(const Map& m, const View::ViewPort& v, const vec2d& screen_pos);
void getVisibleTileRect(const Map& m, const View::ViewPort& v, vec2i& tl, vec2i& br);