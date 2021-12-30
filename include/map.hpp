#pragma once

#include "view.hpp"
#include "tilemap.hpp"

#include <vector>
#include <bitset>
#include <memory>

struct Tile
{
	int id = -1;
	vec2i pos;
};

class Map
{
public:
	Map();
	~Map();

	bool isEditable()
	{
		return editable;
	}

	void draw(const View& v, bool grid);
	void sortMapVisibilty(const View& v);
	void getVisibleTileRect(const View& v, vec2i& tl, vec2i& br);

	void setTilemap(std::string path, vec2i tile_size);
	void insertTile(Tile t);
	void removeTile(const vec2i& p);
	vec2i getTilePos(const View& v, const vec2f& screen_pos);
	Tile getTile(const vec2i& p);

	void create(std::string tilemap_path, vec2i tile_size);
	bool save(std::string file, const View& v);
	bool load(std::string file, View& v, bool restore_view);

	std::vector<Tile> v_tiles;
	std::vector<Tile>::iterator it_visible_begin;
	std::unique_ptr<Tilemap> tilemap;

private:
	bool editable;
};