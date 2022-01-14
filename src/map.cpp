#include "map.hpp"

#include <iostream> // for debugging
#include <math.h> // floor

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

bool createMap(Map& m, std::string path, int ts)
{
	if (m.bmp != nullptr)
	{
		std::cerr << "Map: " << path << " already loaded." << std::endl;
		return true;
	}

	m.bmp = nullptr;
	m.width = 0;
	m.height = 0;
	m.path = path;
	m.tile_size = ts;

	m.bmp = al_load_bitmap(m.path.c_str());

	if (!m.bmp)
	{
		std::cerr << "Failed to load map: " << m.path << std::endl;
		return false;
	}

	m.width = al_get_bitmap_width(m.bmp) / m.tile_size;
	m.height = al_get_bitmap_height(m.bmp) / m.tile_size;

	m.v_tiles.resize(m.width * m.height, false);

	return true;
}

void clearMap(Map& m)
{
	if (m.bmp)
	{
		al_destroy_bitmap(m.bmp);
		m.bmp = nullptr;
	}

	m.width = 0;
	m.height = 0;
	m.path = "";
	m.tile_size = 0;

	m.v_tiles.clear();
}

bool reloadMap(Map& m)
{
	if (m.bmp)
	{
		al_destroy_bitmap(m.bmp);
		m.bmp = nullptr;
	}

	return createMap(m, m.path, m.tile_size);
}

bool saveMap(const Map& m, std::string file, const View& v)
{
	
	return false;
}

bool loadMap(Map& m, std::string file, View &v, bool restore_view)
{
	
	return false;
}

void drawMap(const Map& m, const View& v, bool draw_grid, bool show_hidden)
{
	vec2i vis_tl, vis_br;
	getVisibleTileRect(m, v, vis_tl, vis_br);

	al_hold_bitmap_drawing(true);

	for (int x = vis_tl.x; x <= vis_br.x; ++x)
	{
		for (int y = vis_tl.y; y <= vis_br.y; ++y)
		{
			if (m.v_tiles[y * m.width + x]) drawBitmapRegion(v, m.bmp, vec2f{x * m.tile_size, y * m.tile_size}, vec2f{m.tile_size, m.tile_size}, vec2f{x * m.tile_size, y * m.tile_size}, 0);
			else if (show_hidden) drawTintedBitmapRegion(v, m.bmp, vec2f{x * m.tile_size, y * m.tile_size}, vec2f{m.tile_size, m.tile_size}, vec2f{x * m.tile_size, y * m.tile_size}, al_map_rgba(100, 100, 100, 100), 0);
		}
	}

	al_hold_bitmap_drawing(false);

	if (draw_grid)
	{
		for (int x = vis_tl.x; x <= vis_br.x + 1; ++x)
		{
			drawLine(v, {x * m.tile_size, vis_tl.y * m.tile_size}, {x * m.tile_size, vis_br.y * m.tile_size + m.tile_size}, al_map_rgb(40, 40, 40), 1);
		}

		for (int y = vis_tl.y; y <= vis_br.y + 1; ++y)
		{
			drawLine(v, {vis_tl.x * m.tile_size, y * m.tile_size}, {vis_br.x * m.tile_size + m.tile_size, y * m.tile_size}, al_map_rgb(40, 40, 40), 1);
		}
	}
}

void hideTile(Map &m, const vec2i& p)
{
	setTile(m, p, false);
}
void showTile(Map &m, const vec2i& p)
{
	setTile(m, p, true);
}
void setTile(Map &m, const vec2i& p, bool show)
{
	if (p.x >= 0 && p.x < m.width && p.y >= 0 && p.y < m.height)
	{
		int index = p.y * m.width + p.x;

		m.v_tiles[index] = show;
	}
}
bool isTileShown(const Map& m, const vec2i& p)
{
	if (p.x >= 0 && p.x < m.width && p.y >= 0 && p.y < m.height)
	{
		int index = p.y * m.width + p.x;

		return m.v_tiles[index];
	}

	return false;
}
void getVisibleTileRect(const Map& m, const View& v, vec2i& tl, vec2i& br)
{
	tl.x = std::max((int)floor((v.world_pos.x - (v.size.x / 2 / v.scale.x)) / m.tile_size), 0);
	tl.y = std::max((int)floor((v.world_pos.y - (v.size.y / 2 / v.scale.y)) / m.tile_size), 0);

	br.x = std::min((int)floor((v.world_pos.x + (v.size.x / 2 / v.scale.x)) / m.tile_size), m.width - 1);
	br.y = std::min((int)floor((v.world_pos.y + (v.size.y / 2 / v.scale.y)) / m.tile_size), m.height - 1);
}

vec2i getTilePos(const Map& m, const View& v, const vec2f& screen_pos)
{
	vec2f p = screenToWorld(screen_pos, v);
	vec2i n;

	n.x = (int)floor(p.x / m.tile_size);
	n.y = (int)floor(p.y / m.tile_size);

	return n;
}


