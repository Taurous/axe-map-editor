#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include "command.hpp"

#include "vec.hpp"
#include "map.hpp"

class SetTileCommand : public Command
{
public:
	SetTileCommand(Map& map, const vec2i& position, const std::string& label) : m(map) {
		prev_t = map.getTile(position);
		new_t.position = position;
		new_t.label = label;
		redo();
	}
	void redo() override { m.setTile(new_t.position, new_t.label); std::cout << "SetTileCommand Executed at: " << new_t.position << " with label: " << new_t.label << "\n"; }
	void undo() override {
		if (prev_t.label == "null")
			m.removeTile(new_t.position);
		else
			m.setTile(prev_t.position, prev_t.label);
	}

private:
	Map& m;
	Tile new_t;
	Tile prev_t;
};

class DeleteTileCommand : public Command
{
public:
	DeleteTileCommand(Map& map, const vec2i& position) : m(map) {
		prev_t = map.getTile(position);
		redo();
	}
	void redo() override { m.removeTile(prev_t.position); std::cout << "DeleteTileCommand Executed at: " << prev_t.position << "\n"; }
	void undo() override { m.setTile(prev_t.position, prev_t.label); }

private:
	Map& m;
	Tile prev_t;
};