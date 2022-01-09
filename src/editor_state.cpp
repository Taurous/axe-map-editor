#include <iostream>
#include <math.h>
#include <functional>

#include "editor_state.hpp"

#include "input.hpp"
#include "state_machine.hpp"
#include "util.hpp"

constexpr int SIDEBAR_WIDTH = 256;
constexpr int BOTTOM_BAR_HEIGHT = 40;
constexpr size_t UNDO_STACK_LIMIT = 50;

constexpr bool SAVE_VIEW = false;

void resizeView(View &v)
{
	vec2f screen_size = getScreenSize();

	v.size.x = screen_size.x - SIDEBAR_WIDTH;
	v.size.y = screen_size.y - BOTTOM_BAR_HEIGHT;
}

EditorState::EditorState(StateMachine& state_machine, InputHandler& input)
	: AbstractState(state_machine, input), fn(nullptr), ts(map, view), dragging(false), filling(false), draw_grid(true)
{
	fn = al_load_font("resources/tex/Retro Gaming.ttf", 18, 0);

	view.world_pos = { 0.f, 0.f };
	view.screen_pos = { 0.f, 0.f };
	view.scale = { 1.f, 1.f };

	resizeView(view);

	map.create("resources/tex/Dark_lvl0R.png", { 64, 64 });
}

EditorState::~EditorState()
{

}

void EditorState::pause()
{

}

void EditorState::resume()
{
	m_input.setKeybind(MOUSE::WHEELUP, 	std::bind(&EditorState::onMouseWheelUp, this));
	m_input.setKeybind(MOUSE::WHEELDOWN, std::bind(&EditorState::onMouseWheelDown, this));
	m_input.setKeybind(MOUSE::MIDDLE, 	std::bind(&EditorState::onMiddleMouseDown, this));
	m_input.setKeybind(MOUSE::MIDDLE, 	std::bind(&EditorState::onMiddleMouseUp, this), false);
	m_input.setKeybind(MOUSE::LEFT, 		std::bind(&EditorState::onLeftMouseDown, this));
	m_input.setKeybind(MOUSE::LEFT, 		std::bind(&EditorState::onLeftMouseUp, this), false);
	m_input.setKeybind(MOUSE::RIGHT, 	std::bind(&EditorState::onRightMouseDown, this));
	m_input.setKeybind(MOUSE::RIGHT, 	std::bind(&EditorState::onRightMouseUp, this), false);
	m_input.setKeybind(ALLEGRO_KEY_G,			[&](){ draw_grid = !draw_grid; });
	m_input.setKeybind(ALLEGRO_KEY_Z, 			std::bind(&EditorState::undo, this));
	m_input.setKeybind(ALLEGRO_KEY_Y, 			std::bind(&EditorState::redo, this));
	m_input.setKeybind(ALLEGRO_KEY_S, 			std::bind(&EditorState::saveMap, this));
	m_input.setKeybind(ALLEGRO_KEY_L, 			std::bind(&EditorState::loadMap, this));
}

void EditorState::handleEvents(const ALLEGRO_EVENT &ev)
{
	if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
	{
		resizeView(view);
		map.sortMapVisibilty(view);
	}
}

void EditorState::update(float delta_time)
{
	if (!m_input.isMouseDown(MOUSE::MIDDLE) && map.isEditable())
	{
		vec2f direction{ 0,0 };
		float vel = 200.f;
		if (m_input.isKeyDown(ALLEGRO_KEY_LSHIFT))	vel *= 2.5f;

		if (m_input.isKeyDown(ALLEGRO_KEY_LEFT)	|| m_input.isKeyDown(ALLEGRO_KEY_A)) direction.x -= 1.f;
		if (m_input.isKeyDown(ALLEGRO_KEY_RIGHT)|| m_input.isKeyDown(ALLEGRO_KEY_D)) direction.x += 1.f;
		if (m_input.isKeyDown(ALLEGRO_KEY_UP)	|| m_input.isKeyDown(ALLEGRO_KEY_W)) direction.y -= 1.f;
		if (m_input.isKeyDown(ALLEGRO_KEY_DOWN)	|| (m_input.isKeyDown(ALLEGRO_KEY_S) && !m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))) direction.y += 1.f;

		view.world_pos += normalize(direction) * vel * delta_time;
		map.sortMapVisibilty(view);
	}
	else if (dragging)
	{
		vec2f cur_mouse_pos = m_input.getMousePos();
		view.world_pos = last_pos + ((mouse_pos - cur_mouse_pos) / view.scale);
		map.sortMapVisibilty(view);
	}
}

void EditorState::draw()
{
	// View Drawing, clipped

	al_set_clipping_rectangle((int)view.screen_pos.x, (int)view.screen_pos.y, (int)view.size.x, (int)view.size.y);
	map.draw(view, draw_grid);

	if (filling)
	{
		vec2f screen_fill_start = vec2f(fill_start_pos) * map.tilemap->tile_size;
		vec2f screen_fill_end = vec2f(map.getTilePos(view, m_input.getMousePos())) * map.tilemap->tile_size;

		vec2f t_start_fill, t_end_fill;

		t_start_fill.x = std::min(screen_fill_start.x, screen_fill_end.x);
		t_start_fill.y = std::min(screen_fill_start.y, screen_fill_end.y);
		t_end_fill.x = std::max(screen_fill_start.x, screen_fill_end.x);
		t_end_fill.y = std::max(screen_fill_start.y, screen_fill_end.y);

		drawRectangle(view, t_start_fill, t_end_fill + map.tilemap->tile_size, al_map_rgb(255, 0, 0), 1);
	}
	al_reset_clipping_rectangle();

	//Draw UI

	vec2f screen_dim = getScreenSize();

	al_draw_filled_rectangle(view.size.x, 0, screen_dim.x, screen_dim.y, al_map_rgb(64, 64, 64));
	al_draw_filled_rectangle(0, screen_dim.y - BOTTOM_BAR_HEIGHT, view.size.x, screen_dim.y, al_map_rgb(64, 64, 64));
	vec2f cur_mouse_pos = screenToWorld(m_input.getMousePos(), view);
	al_draw_textf(fn, al_map_rgb(0, 0, 0), 16, screen_dim.y - (BOTTOM_BAR_HEIGHT / 2) - 10, 0, "%i:%i",
		(int)floor(cur_mouse_pos.x / map.tilemap->tile_size.x), (int)floor(cur_mouse_pos.y / map.tilemap->tile_size.y));

	// Tilemap selection
	ts.draw({ view.size.x, 0 }, { SIDEBAR_WIDTH, SIDEBAR_WIDTH }, 20, 10);

	//Debug
	al_draw_textf(fn, al_map_rgb(255, 255, 255), 10, 10, 0, "undo stack size: %li", undo_stack.size());
	al_draw_textf(fn, al_map_rgb(255, 255, 255), 10, 28, 0, "redo stack size: %li", redo_stack.size());
	auto num_tiles = std::distance(map.it_visible_begin, map.v_tiles.end());
	al_draw_textf(fn, al_map_rgb(255, 255, 255), 10, 46, 0, "tiles drawn: %i", (int)num_tiles);
	auto size_of_tilearray = map.v_tiles.size();
	al_draw_textf(fn, al_map_rgb(255, 255, 255), 10, 64, 0, "tilearray size: %i", (int)size_of_tilearray);
}

void EditorState::pushCommand(std::unique_ptr<Command> c)
{
	undo_stack.push_back(std::move(c));

	redo_stack.clear();

	//Limit undo stack size
	if (undo_stack.size() > UNDO_STACK_LIMIT) undo_stack.pop_front();
}

void EditorState::onMouseWheelUp()
{
	if (map.isEditable() && view.scale.x <= 3.f)
	{
		view.scale += { 0.1f, 0.1f };
		map.sortMapVisibilty(view);
	}
}
void EditorState::onMouseWheelDown()
{
	if (map.isEditable() && view.scale.x >= 0.4f)
	{
		view.scale -= { 0.1f, 0.1f };
		map.sortMapVisibilty(view);
	}
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
	if (!map.isEditable()) return;
	
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_SHIFT))
	{
		pushCommand(std::make_unique<FillTileCommand>(map, 0, fill_start_pos, map.getTilePos(view, m_input.getMousePos())));
	}

	filling = false;
}
void EditorState::onLeftMouseDown()
{
	if (!map.isEditable()) return;
	
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_SHIFT))
	{
		filling = true;
		fill_start_pos = map.getTilePos(view, m_input.getMousePos());
	}
	else
	{
		if (map.getTile(map.getTilePos(view, m_input.getMousePos())).id != 0) // CHANGE 0 TO SELECTED TILE
		{
			pushCommand(std::make_unique<InsertTileCommand>(map, Tile{ 0, map.getTilePos(view, m_input.getMousePos()) }));
		}
	}
}
void EditorState::onRightMouseDown()
{
	if (!map.isEditable()) return;

	Tile t = map.getTile(map.getTilePos(view, m_input.getMousePos()));
	if (t.id != -1)
	{
		pushCommand(std::make_unique<RemoveTileCommand>(map, t.pos));
		map.sortMapVisibilty(view);
	}
}
void EditorState::onRightMouseUp() { }

void EditorState::saveMap()
{
	if (map.isEditable() && m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		map.save("mapdata/data.xml", view);
		std::cout << "Saved mapdata.xml" << std::endl;
	}
}
void EditorState::loadMap()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		if (map.load("mapdata/data.xml", view, SAVE_VIEW))
		{
			std::cout << "Loaded mapdata.xml" << std::endl;
			undo_stack.clear();
			redo_stack.clear();
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