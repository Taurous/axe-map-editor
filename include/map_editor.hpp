#pragma once

#include <list>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "input.hpp"
#include "view.hpp"
#include "map.hpp"
#include "edit_commands.hpp"

class MapEditor
{
public:
	MapEditor(InputHandler& input, vec2i view_pos, vec2i view_size);
	~MapEditor();

	void handleEvents(const ALLEGRO_EVENT &ev);
	void update(double delta_time);
	void draw();

	bool create(std::string image_path, int tile_size);
	bool save();
	bool load(std::string path);
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

	void addTileToEditVector(vec2i position, bool show);
	bool isMouseInView();
	void zoomToCursor(bool zoom_out);
	void resizeView(vec2i view_pos, vec2i view_size);

	void fireEvent(int event_id);
	ALLEGRO_EVENT_SOURCE *getEventSource();

private: // TODO Reorganize
	InputHandler &m_input;
	ALLEGRO_EVENT_SOURCE m_event_source;

	bool image_loaded;

	View::ViewPort view;
	Map map;

	vec2i last_pos; // Needs better name
	vec2i dragging_start_pos;

	vec2i last_tile_hovered;

	vec2i fill_start_pos;
	bool dragging;
	bool filling;
	bool show_hidden;
	bool draw_grid;

	std::list<std::unique_ptr<Command>> redo_stack;
	std::list<std::unique_ptr<Command>> undo_stack;

	void pushCommand(std::unique_ptr<Command> c);
	std::vector<vec2i> tiles_to_edit;

	void enableKeybinds();
	void disableKeybinds();
};