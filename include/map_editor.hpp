#pragma once

#include <list>
#include <vector>
#include <memory>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "input.hpp"
#include "view.hpp"
#include "map.hpp"
#include "edit_commands.hpp"

class MapEditor
{
public:
	MapEditor(InputHandler* input, vec2i view_pos, vec2i view_size);
	~MapEditor();

	void handleEvents(const ALLEGRO_EVENT &ev);
	void update(double delta_time);
	void draw();

	void undo();
	void redo();
	
	void onMouseWheelUp();
	void onMouseWheelDown();
	void onMiddleMouseUp();
	void onMiddleMouseDown();
	void onLeftMouseUp();
	void onLeftMouseDown();
	void onRightMouseDown();
	void onRightMouseUp();

	bool isMouseInView();
	void zoomToCursor(bool zoom_out);
	void resizeView(vec2i view_pos, vec2i view_size);

private: // TODO Reorganize
	InputHandler *m_input;
	View::ViewPort view;
	Map map;

	bool drawing_line;
	vec2d line_begin;
	double hover_radius;

	bool dragging;
	vec2i dragging_mouse_begin;
	vec2d dragging_world_begin;
	
	bool draw_grid;

	bool hovering_vertex;
	vec2i hovered_vertex;

	std::list<std::unique_ptr<Command>> redo_stack;
	std::list<std::unique_ptr<Command>> undo_stack;

	void pushCommand(std::unique_ptr<Command> c);
	void enableKeybinds();
	void disableKeybinds();
};