#include <iostream>
#include <math.h>
#include <functional>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_native_dialog.h>

#include "map_editor.hpp"
#include "util.hpp"
#include "editor_events.hpp"

constexpr int BOTTOM_BAR_HEIGHT = 64;
constexpr size_t UNDO_STACK_LIMIT = 50;
constexpr double MIN_ZOOM = 0.13;
constexpr double MAX_ZOOM = 2.19;
constexpr double ZOOM_FACTOR = 0.08;

constexpr bool SAVE_VIEW = true;

void MapEditor::resizeView(vec2i view_pos, vec2i view_size)
{
	view.size = view_size;
	view.screen_pos = view_pos;
}

MapEditor::MapEditor(InputHandler& input, ALLEGRO_EVENT_SOURCE& event_source, vec2i view_pos, vec2i view_size)
	: m_input(input), m_event_source(event_source), dragging(false), filling(false), show_hidden(false), draw_grid(true)
{
	createMap(map, "/home/aksel/Downloads/Maps of the Mad Mage-20220114T044344Z-001/Maps of the Mad Mage/L1_grid.jpg", 100);
	save_file = {"mapdata.bin"};

	view.world_pos = { 0.0, 0.0 };
	view.scale = { 1.0, 1.0 };

	resizeView(view_pos, view_size);

	last_tile_hovered = { -1, -1 };

	m_input.setKeybind(MOUSE::WHEELUP, 		[this](){ onMouseWheelUp(); });
	m_input.setKeybind(MOUSE::WHEELDOWN,	[this](){ onMouseWheelDown(); });
	m_input.setKeybind(MOUSE::MIDDLE, 		[this](){ onMiddleMouseDown(); });
	m_input.setKeybind(MOUSE::MIDDLE, 		[this](){ onMiddleMouseUp(); }, false);
	m_input.setKeybind(MOUSE::LEFT, 		[this](){ onLeftMouseDown(); });
	m_input.setKeybind(MOUSE::LEFT, 		[this](){ onLeftMouseUp(); }, false);
	m_input.setKeybind(MOUSE::RIGHT, 		[this](){ onRightMouseDown(); });
	m_input.setKeybind(MOUSE::RIGHT, 		[this](){ onRightMouseUp(); }, false);
	m_input.setKeybind(ALLEGRO_KEY_G,		[this](){ draw_grid = !draw_grid; });
	m_input.setKeybind(ALLEGRO_KEY_Z, 		[this](){ undo(); });
	m_input.setKeybind(ALLEGRO_KEY_Y, 		[this](){ redo(); });
	m_input.setKeybind(ALLEGRO_KEY_S, 		[this](){ if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL)) save(); });
	m_input.setKeybind(ALLEGRO_KEY_L, 		[this](){ if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL)) load(); });
	m_input.setKeybind(ALLEGRO_KEY_C,		[this](){ if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL)) view.world_pos = { 0, 0 }; });
	m_input.setKeybind(ALLEGRO_KEY_R,		[this](){ view.scale = {1, 1}; });
	m_input.setKeybind(ALLEGRO_KEY_SPACE,	[this](){ show_hidden = !show_hidden; });
	m_input.setKeybind(ALLEGRO_KEY_UP,		[this](){
		ALLEGRO_EVENT editor_event;
		editor_event.user.type = AXE_EDITOR_EVENT_SCALE_VIEW;
		editor_event.user.data1 = -1;
		al_emit_user_event(&m_event_source, &editor_event, nullptr);
	});
	m_input.setKeybind(ALLEGRO_KEY_DOWN,	[this](){
		ALLEGRO_EVENT editor_event;
		editor_event.user.type = AXE_EDITOR_EVENT_SCALE_VIEW;
		editor_event.user.data1 = 1;
		al_emit_user_event(&m_event_source, &editor_event, nullptr);
	});
}

MapEditor::~MapEditor()
{
	al_destroy_bitmap(map.bmp);
}

void MapEditor::handleEvents(const ALLEGRO_EVENT &ev)
{
	vec2i cur_tile_hovered;
	vec2i mouse = m_input.getMousePos();

	switch (ev.type)
	{
		case ALLEGRO_EVENT_MOUSE_AXES:
			cur_tile_hovered = getTilePos(map, view, mouse);

			if (isMouseInView()) // Test
			{
				if (m_input.isMouseDown(MOUSE::LEFT) && !filling)
				{
					if (cur_tile_hovered != last_tile_hovered)
					{
						if (!isTileShown(map, cur_tile_hovered)) addTileToEditVector(cur_tile_hovered, true);
					}
				}
				else if (m_input.isMouseDown(MOUSE::RIGHT) && !filling)
				{
					if (cur_tile_hovered != last_tile_hovered)
					{
						if (isTileShown(map, cur_tile_hovered)) addTileToEditVector(cur_tile_hovered, false);
					}
				}

				if (dragging) view.world_pos = vec2d(last_pos) + (vec2d(mouse_pos - m_input.getMousePos()) / view.scale);
			}
			last_tile_hovered = getTilePos(map, view, mouse);
		break;

		default:
		break;
	}	
}

void MapEditor::update(double delta_time)
{
	if (!m_input.isMouseDown(MOUSE::MIDDLE))
	{
		vec2d direction{ 0,0 };
		double vel = 500.0;
		if (m_input.isKeyDown(ALLEGRO_KEY_LSHIFT))	vel *= 2.5;

		if (m_input.isKeyDown(ALLEGRO_KEY_A)) direction.x -= 1.0;
		if (m_input.isKeyDown(ALLEGRO_KEY_D)) direction.x += 1.0;
		if (m_input.isKeyDown(ALLEGRO_KEY_W)) direction.y -= 1.0;
		if ((m_input.isKeyDown(ALLEGRO_KEY_S) && !m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))) direction.y += 1.0;

		view.world_pos += normalize(direction) * vel * delta_time;
	}
}

void MapEditor::draw()
{
	// View Drawing, clipped
	al_set_clipping_rectangle((int)view.screen_pos.x, (int)view.screen_pos.y, (int)view.size.x, (int)view.size.y);

	drawMap(map, view, draw_grid, show_hidden);

	if (filling)
	{
		vec2i screen_fill_start = fill_start_pos * map.tile_size;
		vec2i screen_fill_end = getTilePos(map, view, m_input.getMousePos()) * map.tile_size;

		vec2i t_start_fill, t_end_fill;

		t_start_fill.x = std::min(screen_fill_start.x, screen_fill_end.x);
		t_start_fill.y = std::min(screen_fill_start.y, screen_fill_end.y);
		t_end_fill.x = std::max(screen_fill_start.x, screen_fill_end.x);
		t_end_fill.y = std::max(screen_fill_start.y, screen_fill_end.y);

		drawRectangle(view, t_start_fill, t_end_fill + vec2i{map.tile_size, map.tile_size}, al_map_rgb(255, 0, 0), 1);
	}

	al_reset_clipping_rectangle();
}

void MapEditor::pushCommand(std::unique_ptr<Command> c)
{
	undo_stack.push_back(std::move(c));

	redo_stack.clear();

	//Limit undo stack size
	if (undo_stack.size() > UNDO_STACK_LIMIT) undo_stack.pop_front();
}

void MapEditor::onMouseWheelUp()
{
	if (isMouseInView()) zoomToCursor(false);
}
void MapEditor::onMouseWheelDown()
{
	if (isMouseInView()) zoomToCursor(true);
}
void MapEditor::onMiddleMouseUp()
{
	dragging = false;
}
void MapEditor::onMiddleMouseDown()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		ALLEGRO_EVENT editor_event;
		editor_event.user.type = AXE_EDITOR_EVENT_MOVE_VIEW;
		auto new_pos = screenToWorld(m_input.getMousePos(), view);
		editor_event.user.data1 = new_pos.x;
		editor_event.user.data2 = new_pos.y;
		al_emit_user_event(&m_event_source, &editor_event, nullptr);
		return;
	}
	
	if (isMouseInView())
	{
		mouse_pos = m_input.getMousePos();
		last_pos = view.world_pos;
		dragging = true;
	}
}
void MapEditor::onLeftMouseUp()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_SHIFT) && filling)
	{
		pushCommand(std::make_unique<FillTileCommand>(map, true, fill_start_pos, getTilePos(map, view, m_input.getMousePos())));
	}
	else if (!tiles_to_edit.empty())
	{
		pushCommand(std::make_unique<SetTileCommand>(map, tiles_to_edit, true));
		tiles_to_edit.clear();
	}

	filling = false;
}
void MapEditor::onLeftMouseDown()
{
	if (!isMouseInView()) return;

	if (m_input.isModifierDown(ALLEGRO_KEYMOD_SHIFT) && !filling)
	{
		filling = true;
		fill_start_pos = getTilePos(map, view, m_input.getMousePos());
	}
	else if (!isTileShown(map, getTilePos(map, view, m_input.getMousePos())))
	{
		addTileToEditVector(getTilePos(map, view, m_input.getMousePos()), true);
	}
}

void MapEditor::onRightMouseUp()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_SHIFT) && filling)
	{
		pushCommand(std::make_unique<FillTileCommand>(map, false, fill_start_pos, getTilePos(map, view, m_input.getMousePos())));
	}
	else if (!tiles_to_edit.empty())
	{
		pushCommand(std::make_unique<SetTileCommand>(map, tiles_to_edit, false));
		tiles_to_edit.clear();
	}

	filling = false;
}
void MapEditor::onRightMouseDown()
{
	if (!isMouseInView()) return;
	
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_SHIFT) && !filling)
	{
		filling = true;
		fill_start_pos = getTilePos(map, view, m_input.getMousePos());
	}
	else if (isTileShown(map, getTilePos(map, view, m_input.getMousePos())))
	{
		addTileToEditVector(getTilePos(map, view, m_input.getMousePos()), false);
	}
}

void MapEditor::addTileToEditVector(vec2i position, bool show)
{
	tiles_to_edit.push_back(position);
	setTile(map, position, show);
}

void MapEditor::save()
{
	saveMap(map, "mapdata/data.bin", view);
}
void MapEditor::load()
{
	if (loadMap(map, "mapdata/data.bin", view, SAVE_VIEW))
	{
		undo_stack.clear();
		redo_stack.clear();
	}
}

void MapEditor::undo()
{
	if (!undo_stack.empty() && m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		Command *c = undo_stack.back().release();
		undo_stack.pop_back();

		c->undo();
		
		redo_stack.push_back(std::unique_ptr<Command>(c));
	}
}

void MapEditor::redo()
{
	if (!redo_stack.empty() && m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		Command *c = redo_stack.back().release();
		redo_stack.pop_back();

		c->redo();

		undo_stack.push_back(std::unique_ptr<Command>(c));
	}
}

void MapEditor::zoomToCursor(bool zoom_out)
{
	vec2d prev_pos = screenToWorld(m_input.getMousePos(), view);

	if (zoom_out)
	{
		if (view.scale.x > MIN_ZOOM)
		{
			view.scale -= { ZOOM_FACTOR, ZOOM_FACTOR };
		}
	}
	else
	{
		if (view.scale.x < MAX_ZOOM)
		{
			view.scale += { ZOOM_FACTOR, ZOOM_FACTOR };
		}
	}

	vec2d new_pos = screenToWorld(m_input.getMousePos(), view);
	view.world_pos += prev_pos - new_pos;
}

bool MapEditor::isMouseInView()
{
	return m_input.getMousePos().isInBounds(view.screen_pos, view.screen_pos + view.size);
}

vec2i MapEditor::getViewPosition()
{
	return view.world_pos;
}