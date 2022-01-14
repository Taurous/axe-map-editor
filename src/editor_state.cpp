#include <iostream>
#include <math.h>
#include <functional>
#include <allegro5/allegro_color.h>

#include "editor_state.hpp"

#include "input.hpp"
#include "state_machine.hpp"
#include "util.hpp"

constexpr int BOTTOM_BAR_HEIGHT = 48;
constexpr size_t UNDO_STACK_LIMIT = 50;
constexpr float MIN_ZOOM = 0.1f;
constexpr float MAX_ZOOM = 4.f;

constexpr bool SAVE_VIEW = true;

void resizeView(View &v)
{
	vec2f screen_size = getScreenSize();

	v.size.x = screen_size.x;
	v.size.y = screen_size.y - BOTTOM_BAR_HEIGHT;
}

EditorState::EditorState(StateMachine& state_machine, InputHandler& input)
	: AbstractState(state_machine, input), fn(nullptr), dragging(false), filling(false), show_hidden(false), saved(true), draw_grid(true), draw_debug(false)
{
	fn = al_load_font("resources/tex/Retro Gaming.ttf", 22, 0);

	createMap(map, "/home/aksel/Downloads/Maps of the Mad Mage-20220114T044344Z-001/Maps of the Mad Mage/L1_grid.jpg", 100);

	view.world_pos = { 0.f, 0.f };
	view.screen_pos = { 0.f, 0.f };
	view.scale = { 1.f, 1.f };

	resizeView(view);

	last_tile_hovered = { -1, -1 };
}

EditorState::~EditorState()
{
	al_destroy_font(fn);
}

void EditorState::pause()
{
	m_input.clearKeybind(MOUSE::WHEELUP);
	m_input.clearKeybind(MOUSE::WHEELDOWN);
	m_input.clearKeybind(MOUSE::MIDDLE);
	m_input.clearKeybind(MOUSE::LEFT);
	m_input.clearKeybind(MOUSE::RIGHT);
	m_input.clearKeybind(ALLEGRO_KEY_G);
	m_input.clearKeybind(ALLEGRO_KEY_Z);
	m_input.clearKeybind(ALLEGRO_KEY_Y);
	m_input.clearKeybind(ALLEGRO_KEY_S);
	m_input.clearKeybind(ALLEGRO_KEY_L);
	m_input.clearKeybind(ALLEGRO_KEY_F3);
	m_input.clearKeybind(ALLEGRO_KEY_C);
	m_input.clearKeybind(ALLEGRO_KEY_R);
	m_input.clearKeybind(ALLEGRO_KEY_SPACE);
}

void EditorState::resume()
{
	m_input.setKeybind(MOUSE::WHEELUP, 		std::bind(&EditorState::onMouseWheelUp, this));
	m_input.setKeybind(MOUSE::WHEELDOWN,	std::bind(&EditorState::onMouseWheelDown, this));
	m_input.setKeybind(MOUSE::MIDDLE, 		std::bind(&EditorState::onMiddleMouseDown, this));
	m_input.setKeybind(MOUSE::MIDDLE, 		std::bind(&EditorState::onMiddleMouseUp, this), false);
	m_input.setKeybind(MOUSE::LEFT, 		std::bind(&EditorState::onLeftMouseDown, this));
	m_input.setKeybind(MOUSE::LEFT, 		std::bind(&EditorState::onLeftMouseUp, this), false);
	m_input.setKeybind(MOUSE::RIGHT, 		std::bind(&EditorState::onRightMouseDown, this));
	m_input.setKeybind(MOUSE::RIGHT, 		std::bind(&EditorState::onRightMouseUp, this), false);
	m_input.setKeybind(ALLEGRO_KEY_G,		[&](){ draw_grid = !draw_grid; });
	m_input.setKeybind(ALLEGRO_KEY_Z, 		std::bind(&EditorState::undo, this));
	m_input.setKeybind(ALLEGRO_KEY_Y, 		std::bind(&EditorState::redo, this));
	m_input.setKeybind(ALLEGRO_KEY_S, 		std::bind(&EditorState::save, this));
	m_input.setKeybind(ALLEGRO_KEY_L, 		std::bind(&EditorState::load, this));
	m_input.setKeybind(ALLEGRO_KEY_F3,		[&](){ draw_debug = !draw_debug; });
	m_input.setKeybind(ALLEGRO_KEY_C,		[&](){ if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL)) view.world_pos = { 0, 0 }; });
	m_input.setKeybind(ALLEGRO_KEY_R,		[&](){ view.scale = {1, 1}; });
	m_input.setKeybind(ALLEGRO_KEY_SPACE,	[&](){ show_hidden = !show_hidden; });
}

void EditorState::handleEvents(const ALLEGRO_EVENT &ev)
{
	vec2i cur_tile_hovered;

	switch (ev.type)
	{
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
			resizeView(view);
		break;

		case ALLEGRO_EVENT_MOUSE_AXES:
			cur_tile_hovered = getTilePos(map, view, m_input.getMousePos());

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
			last_tile_hovered = getTilePos(map, view, m_input.getMousePos());
		break;

		default:
		break;
	}	
}

void EditorState::update(float delta_time)
{
	if (!m_input.isMouseDown(MOUSE::MIDDLE))
	{
		vec2f direction{ 0,0 };
		float vel = 200.f;
		if (m_input.isKeyDown(ALLEGRO_KEY_LSHIFT))	vel *= 2.5f;

		if (m_input.isKeyDown(ALLEGRO_KEY_LEFT)	|| m_input.isKeyDown(ALLEGRO_KEY_A)) direction.x -= 1.f;
		if (m_input.isKeyDown(ALLEGRO_KEY_RIGHT)|| m_input.isKeyDown(ALLEGRO_KEY_D)) direction.x += 1.f;
		if (m_input.isKeyDown(ALLEGRO_KEY_UP)	|| m_input.isKeyDown(ALLEGRO_KEY_W)) direction.y -= 1.f;
		if (m_input.isKeyDown(ALLEGRO_KEY_DOWN)	|| (m_input.isKeyDown(ALLEGRO_KEY_S) && !m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))) direction.y += 1.f;

		view.world_pos += normalize(direction) * vel * delta_time;
	}
	else if (dragging)
	{
		vec2f cur_mouse_pos = m_input.getMousePos();
		view.world_pos = last_pos + ((mouse_pos - cur_mouse_pos) / view.scale);
	}
}

void EditorState::draw()
{
	// View Drawing, clipped

	al_set_clipping_rectangle((int)view.screen_pos.x, (int)view.screen_pos.y, (int)view.size.x, (int)view.size.y);

	drawMap(map, view, draw_grid, show_hidden);

	if (filling)
	{
		vec2f screen_fill_start = vec2f(fill_start_pos) * static_cast<float>(map.tile_size);
		vec2f screen_fill_end = vec2f(getTilePos(map, view, m_input.getMousePos())) * static_cast<float>(map.tile_size);

		vec2f t_start_fill, t_end_fill;

		t_start_fill.x = std::min(screen_fill_start.x, screen_fill_end.x);
		t_start_fill.y = std::min(screen_fill_start.y, screen_fill_end.y);
		t_end_fill.x = std::max(screen_fill_start.x, screen_fill_end.x);
		t_end_fill.y = std::max(screen_fill_start.y, screen_fill_end.y);

		drawRectangle(view, t_start_fill, t_end_fill + vec2f{ static_cast<float>(map.tile_size), static_cast<float>(map.tile_size) }, al_map_rgb(255, 0, 0), 1);
	}

	al_reset_clipping_rectangle();

	//Draw UI

	vec2f screen_dim = getScreenSize();
	vec2i world_tile = view.world_pos / vec2f{map.tile_size, map.tile_size};

	al_draw_filled_rectangle(0, screen_dim.y - BOTTOM_BAR_HEIGHT, screen_dim.x, screen_dim.y, al_color_html("#d35400"));

	al_draw_textf(fn, al_map_rgb(0, 0, 0), 100, screen_dim.y - (BOTTOM_BAR_HEIGHT / 2), ALLEGRO_ALIGN_CENTER, "Mouse: %i:%i",
		last_tile_hovered.x, last_tile_hovered.y);
	al_draw_textf(fn, al_map_rgb(0, 0, 0), 280, screen_dim.y - (BOTTOM_BAR_HEIGHT / 2), ALLEGRO_ALIGN_CENTER, "View: %i:%i",
		world_tile.x, world_tile.y);
	if (!saved) al_draw_text(fn, al_map_rgb(255, 255, 255), screen_dim.x - 64, screen_dim.y - (BOTTOM_BAR_HEIGHT / 2), ALLEGRO_ALIGN_CENTER, "*");

	//Debug
	if (draw_debug)
	{
		al_draw_textf(fn, al_map_rgb(255, 255, 255), 10, 10, 0, "undo stack size: %li", undo_stack.size());
		al_draw_textf(fn, al_map_rgb(255, 255, 255), 10, 28, 0, "redo stack size: %li", redo_stack.size());
	}

	/* Draw reticle in center of view

	al_draw_line(view.screen_pos.x + (view.size.x / 2.f) - 8,
	view.screen_pos.y + (view.size.y / 2.f),
	view.screen_pos.x + (view.size.x / 2.f) + 8,
	view.screen_pos.y + (view.size.y / 2.f),
	al_map_rgb(255, 0, 0), 2);

	al_draw_line(view.screen_pos.x + (view.size.x / 2.f),
	view.screen_pos.y + (view.size.y / 2.f) - 8,
	view.screen_pos.x + (view.size.x / 2.f),
	view.screen_pos.y + (view.size.y / 2.f) + 8,
	al_map_rgb(255, 0, 0), 2);
	*/
}

void EditorState::pushCommand(std::unique_ptr<Command> c)
{
	saved = false;
	undo_stack.push_back(std::move(c));

	redo_stack.clear();

	//Limit undo stack size
	if (undo_stack.size() > UNDO_STACK_LIMIT) undo_stack.pop_front();
}

void EditorState::onMouseWheelUp()
{
	zoomToCursor(false);
}
void EditorState::onMouseWheelDown()
{
	zoomToCursor(true);
}
void EditorState::onMiddleMouseUp()
{
	dragging = false;
}
void EditorState::onMiddleMouseDown()
{
	mouse_pos = m_input.getMousePos();
	last_pos = view.world_pos;
	dragging = true;
}
void EditorState::onLeftMouseUp()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_SHIFT) && filling)
	{
		pushCommand(std::make_unique<FillTileCommand>(map, true, fill_start_pos, getTilePos(map, view, m_input.getMousePos())));
	}
	else
	{
		if (!tiles_to_edit.empty()) pushCommand(std::make_unique<SetTileCommand>(map, tiles_to_edit, true));
		tiles_to_edit.clear();
	}

	filling = false;
}
void EditorState::onLeftMouseDown()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_SHIFT))
	{
		filling = true;
		fill_start_pos = getTilePos(map, view, m_input.getMousePos());
	}
	else if (!isTileShown(map, getTilePos(map, view, m_input.getMousePos())))
	{
		addTileToEditVector(getTilePos(map, view, m_input.getMousePos()), true);
	}
}
void EditorState::onRightMouseDown()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_SHIFT))
	{
		filling = true;
		fill_start_pos = getTilePos(map, view, m_input.getMousePos());
	}
	else if (isTileShown(map, getTilePos(map, view, m_input.getMousePos())))
	{
		addTileToEditVector(getTilePos(map, view, m_input.getMousePos()), false);
	}
}
void EditorState::onRightMouseUp()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_SHIFT) && filling)
	{
		pushCommand(std::make_unique<FillTileCommand>(map, false, fill_start_pos, getTilePos(map, view, m_input.getMousePos())));
	}
	else
	{
		if (!tiles_to_edit.empty()) pushCommand(std::make_unique<SetTileCommand>(map, tiles_to_edit, false));
		tiles_to_edit.clear();
	}

	filling = false;
}

void EditorState::addTileToEditVector(vec2i position, bool show)
{
	tiles_to_edit.push_back(position);
	setTile(map, position, show);
}

void EditorState::save()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		if (saveMap(map, "mapdata/data.xml", view)) saved = true;
	}
}
void EditorState::load()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		if (loadMap(map, "mapdata/data.xml", view, SAVE_VIEW))
		{
			undo_stack.clear();
			redo_stack.clear();
			saved = true;
		}
	}
}

void EditorState::undo()
{
	if (!undo_stack.empty() && m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		Command *c = undo_stack.back().release();
		undo_stack.pop_back();

		c->undo();
		
		redo_stack.push_back(std::unique_ptr<Command>(c));
	}
}

void EditorState::redo()
{
	if (!redo_stack.empty() && m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		Command *c = redo_stack.back().release();
		redo_stack.pop_back();

		c->redo();

		undo_stack.push_back(std::unique_ptr<Command>(c));
	}
}

void EditorState::zoomToCursor(bool zoom_out)
{
	vec2f prev_pos = screenToWorld(m_input.getMousePos(), view);

	if (zoom_out)
	{
		if (view.scale.x > MIN_ZOOM)
		{
			view.scale -= { 0.1f, 0.1f };
			if (view.scale.x <= 0.001f) view.scale.x = MIN_ZOOM;
			if (view.scale.y <= 0.001f) view.scale.y = MIN_ZOOM;
		}
	}
	else
	{
		if (view.scale.x < MAX_ZOOM)
		{
			view.scale += { 0.1f, 0.1f };
		}
	}

	vec2f new_pos = screenToWorld(m_input.getMousePos(), view);
	view.world_pos += prev_pos - new_pos;
}