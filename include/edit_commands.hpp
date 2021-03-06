#pragma once

#include <vector>
#include <memory>

#include "command.hpp"

#include "vec.hpp"
#include "map.hpp"

class SetTileCommand : public Command
{
public:
	SetTileCommand(Map& map, vec2i position, bool show) : m(map), s(show) { p.push_back(position); redo(); }
	SetTileCommand(Map& map, std::vector<vec2i> positions, bool show) : m(map), p(positions), s(show) { redo(); }
	void redo() override { for (auto &t : p) setTile(m, t, s); }
	void undo() override { for (auto &t : p) setTile(m, t, !s); }

private:
	Map& m;
	std::vector<vec2i> p;
	bool s;
};

class FillTileCommand : public Command
{
public:
	FillTileCommand(Map& map, bool show, vec2i start_fill, vec2i end_fill) : s_fill(start_fill), e_fill(end_fill)
	{
		std::vector<vec2i> tiles;
		vec2i t_start_fill, t_end_fill;

		t_start_fill.x = std::min(s_fill.x, e_fill.x);
		t_start_fill.y = std::min(s_fill.y, e_fill.y);
		t_end_fill.x = std::max(s_fill.x, e_fill.x);
		t_end_fill.y = std::max(s_fill.y, e_fill.y);

		for (int x = t_start_fill.x; x <= t_end_fill.x; ++x)
		{
			for (int y = t_start_fill.y; y <= t_end_fill.y; ++y)
			{
				if (isTileShown(map, vec2i{x, y}) != show) tiles.push_back(vec2i{x, y});
			}
		}

		cmd = std::make_unique<SetTileCommand>(map, tiles, show);
		redo();
	}

	void redo() override
	{
		cmd->redo();
	}
	void undo() override
	{
		cmd->undo();
	}

private:
	vec2i s_fill;
	vec2i e_fill;

	std::unique_ptr<SetTileCommand> cmd;
};