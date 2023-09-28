#include "map.hpp"

#include <iostream> // for debugging
#include <algorithm>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

constexpr int TILE_SZ = 56;

Map::Map()
{

}

Map::~Map()
{

}

void Map::draw(const View::ViewPort& v, const bool& draw_grid)
{
	vec2i vis_tl, vis_br;
	getVisibleTileRect(v, vis_tl, vis_br);

	if (draw_grid) drawGrid(v);

	for (auto &t : tiles)
	{
		if (t.position.isInBounds(vis_tl, vis_br))
		{
			View::drawRectangle(v, t.position * vec2i{ TILE_SZ, TILE_SZ }, t.position * vec2i{ TILE_SZ, TILE_SZ } + vec2i{ TILE_SZ, TILE_SZ }, al_map_rgb(150, 40, 40), 4);
		}
	}
	
	for (auto &l : lines)
	{
		if (l.start.isInBounds(vis_tl, vis_br) || l.end.isInBounds(vis_tl, vis_br))
		{
			View::drawLine(v, l.start, l.end, al_map_rgb(255, 0, 0), 2);
		}
	}

	
}

void Map::drawGrid(const View::ViewPort& v)
{
	vec2i vis_tl, vis_br;
	getVisibleTileRect(v, vis_tl, vis_br);

	int dash_length = TILE_SZ / 7;

	int vertical_begin = vis_tl.y * TILE_SZ;
	int vertical_end = vis_br.y * TILE_SZ + TILE_SZ;
	int horizontal_begin = vis_tl.x * TILE_SZ;
	int horizontal_end = vis_br.x * TILE_SZ + TILE_SZ;

	int vert_dash_count = (vertical_end - vertical_begin) / dash_length;
	int horiz_dash_count = (horizontal_end - horizontal_begin) / dash_length;

	int step = 0;
	int count = 0;
	while (count < vis_br.x - vis_tl.x + 1)
	{
		for (int y = vertical_begin; y <= vertical_end; y += dash_length * 2)
		{
			if (step > 3)
			{
				step = 0;
				y -= dash_length;
			}
			View::drawLine(v, { horizontal_begin + (count * TILE_SZ), y }, { horizontal_begin + (count * TILE_SZ), y + dash_length }, al_map_rgb(40, 40, 40), 1);
			++step;
		}
		step = 0;
		++count;
	}

	step = 0;
	count = 0;
	while (count < vis_br.y - vis_tl.y + 1)
	{
		for (int x = horizontal_begin; x <= horizontal_end; x += dash_length * 2)
		{
			if (step > 3)
			{
				step = 0;
				x -= dash_length;
			}
			View::drawLine(v, { x, vertical_begin + (count * TILE_SZ) }, { x + dash_length, vertical_begin + (count * TILE_SZ) }, al_map_rgb(40, 40, 40), 1);
			++step;
		}
		step = 0;
		++count;
	}
}

void Map::setTile(const vec2i& position, const std::string& label)
{
	for (auto &t : tiles)
	{
		if (t.position == position)
		{
			t.label = label;
			break;
		}
	}

	//Tile does not exist
	Tile t;
	t.position = position;
	t.label = label;

	tiles.push_back(t);
}

Tile Map::getTile(const vec2i& position)
{
	for (auto &tile : tiles)
	{
		if (tile.position == position)
		{
			return tile;
		}
	}

	// Couldn't find tile with specified position, return null tile
	Tile t;
	t.label = "null";
	t.position = vec2i{INT_MAX, INT_MAX};
	return t;
}

void Map::removeTile(const vec2i& position)
{
	auto it = std::find_if(tiles.begin(), tiles.end(), [&](const Tile& t){ return t.position == position; });

	if (it != tiles.end()) tiles.erase(it);
}

bool Map::tileExists(const vec2i& position)
{
	auto it = std::find_if(tiles.begin(), tiles.end(), [&](const Tile& t){ return t.position == position; });

	return (it != tiles.end());
}

void Map::addLine(const Line& line)
{
	lines.push_back(line);
}

void Map::getVisibleTileRect(const View::ViewPort& v, vec2i& tl, vec2i& br)
{
	tl.x = (int)floor((v.world_pos.x - (v.size.x / 2 / v.scale)) / TILE_SZ);
	tl.y = (int)floor((v.world_pos.y - (v.size.y / 2 / v.scale)) / TILE_SZ);

	br.x = (int)floor((v.world_pos.x + (v.size.x / 2 / v.scale)) / TILE_SZ);
	br.y = (int)floor((v.world_pos.y + (v.size.y / 2 / v.scale)) / TILE_SZ);
}

vec2i Map::getTilePos(const View::ViewPort& v, const vec2d& screen_pos)
{
	vec2d p = View::screenToWorld(screen_pos, v);
	vec2i n;

	n.x = (int)floor(p.x / TILE_SZ);
	n.y = (int)floor(p.y / TILE_SZ);

	return n;
}

const int Map::getTileSz() const
{
	return TILE_SZ;
}