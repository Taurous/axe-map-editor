#pragma once

#include "view.hpp"

#include <vector>

struct Line
{
	vec2i start;
	vec2i end;
};

struct Tile
{
	vec2i position;
	std::string label;
};

class Map
{
public:
	Map();
	~Map();

	void draw(const View::ViewPort& v, const bool& draw_grid);

	void setTile(const vec2i& position, const std::string& label);
	Tile getTile(const vec2i& position);
	void removeTile(const vec2i& position);
	bool tileExists(const vec2i& position);

	void addLine(const Line& line);

	vec2i getTilePos(const View::ViewPort& v, const vec2d& screen_pos);
	void getVisibleTileRect(const View::ViewPort& v, vec2i& tl, vec2i& br);
	const int getTileSz() const;

private:
	std::vector<Line> lines;
	std::vector<Tile> tiles;

	void drawGrid(const View::ViewPort& v);
};

