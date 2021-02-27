#pragma once

#include "command.hpp"

#include "vec.hpp"
#include "map.hpp"

class InsertTileCommand : public Command
{
public:
	InsertTileCommand(Map& map, Tile tile) : m(map), t(tile) { redo(); std::cout << "InsertTileCommand" << std::endl; }

	void redo() override
	{
		m.insertTile(t);
	}
	void undo() override
	{
		m.removeTile(t.pos);
	}

private:
	Map& m;
	Tile t;
};

class RemoveTileCommand : public Command
{
public:
	RemoveTileCommand(Map& map, vec2i pos) : m(map) { t = m.getTile(pos); redo(); std::cout << "RemoveTileCommand" << std::endl; }

	void redo() override
	{
		m.removeTile(t.pos);
	}
	void undo() override
	{
		m.insertTile(t);
	}

private:
	Map& m;
	Tile t;
};

class FillTileCommand : public Command
{
public:
	FillTileCommand(Map& map, int tex_id, vec2i start_fill, vec2i end_fill) : m(map), t_id(tex_id), s_fill(start_fill), e_fill(end_fill) { redo(); std::cout << "FileTileCommand" << std::endl; }

	void redo() override
	{
		vec2i t_start_fill, t_end_fill;

		t_start_fill.x = std::min(s_fill.x, e_fill.x);
		t_start_fill.y = std::min(s_fill.y, e_fill.y);
		t_end_fill.x = std::max(s_fill.x, e_fill.x);
		t_end_fill.y = std::max(s_fill.y, e_fill.y);

		for (int x = t_start_fill.x; x <= t_end_fill.x; ++x)
		{
			for (int y = t_start_fill.y; y <= t_end_fill.y; ++y)
			{
				m.insertTile(Tile{ t_id, {x, y} });
			}
		}
	}
	void undo() override
	{
		// TODO
	}

private:
	Map& m;
	int t_id;
	vec2i s_fill;
	vec2i e_fill;
};