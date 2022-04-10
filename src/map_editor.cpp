#include <iostream>
#include <math.h>
#include <functional>
#include <allegro5/allegro_color.h>

#include "map_editor.hpp"
#include "util.hpp"
#include "editor_events.hpp"

constexpr int BOTTOM_BAR_HEIGHT = 64;
constexpr size_t UNDO_STACK_LIMIT = 50;
constexpr double MIN_ZOOM = 0.13;
constexpr double MAX_ZOOM = 2.19;
constexpr double ZOOM_FACTOR = 0.08;

void MapEditor::resizeView(vec2i view_pos, vec2i view_size)
{
	view.size = view_size;
	view.screen_pos = view_pos;
}

MapEditor::MapEditor(InputHandler &input, vec2i view_pos, vec2i view_size)
	: m_input(input), image_loaded(false), dragging(false), filling(false), show_hidden(false), draw_grid(true)
{
	al_init_user_event_source(&m_event_source);

	view.world_pos = {0.0, 0.0};
	view.scale = 1.0;
	resizeView(view_pos, view_size);

	last_tile_hovered = {-1, -1};
}

bool MapEditor::create(std::string image_path, int tile_size)
{
	// TODO: Very minor differences with MapEditor::load(), combine?
	// TODO: Ask user to save previous map if one was open
	Map temp;
	if (!createMap(temp, image_path, tile_size))
	{
		std::cerr << "Failed to create map from image file: " << image_path << std::endl;
		return false;
	}

	// We have successfully loaded a map
	destroyMap(map);
	map = temp;
	undo_stack.clear();
	redo_stack.clear();

	if (!image_loaded)
	{
		enableKeybinds();
		image_loaded = true;
	}

	return true;
}

MapEditor::~MapEditor()
{
	al_destroy_bitmap(map.bmp);
}

void MapEditor::handleEvents(const ALLEGRO_EVENT &ev)
{
	if (ev.type != ALLEGRO_EVENT_MOUSE_AXES || !image_loaded)
		return; // Ignore every event except for mouse_axes, also do not continue if no image has been loaded

	vec2i cur_mouse_pos = m_input.getMousePos();
	vec2i cur_tile_hovered = getTilePos(map, view, cur_mouse_pos);

	if (!isMouseInView())
		return;

	if (dragging)
		view.world_pos = vec2d(last_pos) + (vec2d(dragging_start_pos - cur_mouse_pos) / view.scale);

	if (cur_tile_hovered == last_tile_hovered || filling)
		return; // No need to proceed if the hovered tile hasn't changed, or if user is holding shift to fill

	// If we get here, then mouse is dragging to continuously edit tiles
	if (m_input.isMouseDown(MOUSE::LEFT))
	{
		if (!isTileShown(map, cur_tile_hovered))
			addTileToEditVector(cur_tile_hovered, true);
	}
	else if (m_input.isMouseDown(MOUSE::RIGHT))
	{
		if (isTileShown(map, cur_tile_hovered))
			addTileToEditVector(cur_tile_hovered, false);
	}

	last_tile_hovered = getTilePos(map, view, cur_mouse_pos);
}

void MapEditor::update(double delta_time)
{
	if (!image_loaded)
		return;

	if (!m_input.isMouseDown(MOUSE::MIDDLE))
	{
		vec2d direction{0, 0};
		double vel = 500.0;
		if (m_input.isKeyDown(ALLEGRO_KEY_LSHIFT))
			vel *= 2.5;

		if (m_input.isKeyDown(ALLEGRO_KEY_A))
			direction.x -= 1.0;
		if (m_input.isKeyDown(ALLEGRO_KEY_D))
			direction.x += 1.0;
		if (m_input.isKeyDown(ALLEGRO_KEY_W))
			direction.y -= 1.0;
		if ((m_input.isKeyDown(ALLEGRO_KEY_S) && !m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL)))
			direction.y += 1.0;

		view.world_pos += normalize(direction) * vel * delta_time;
	}
}

void MapEditor::draw()
{
	if (!image_loaded)
		return;

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

	// Limit undo stack size
	if (undo_stack.size() > UNDO_STACK_LIMIT)
		undo_stack.pop_front();
}

void MapEditor::onMouseWheelUp()
{
	if (isMouseInView())
		zoomToCursor(false);
}
void MapEditor::onMouseWheelDown()
{
	if (isMouseInView())
		zoomToCursor(true);
}
void MapEditor::onMiddleMouseUp()
{
	dragging = false;
}
void MapEditor::onMiddleMouseDown()
{
	if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		fireEvent(AXE_EDITOR_EVENT_MOVE_VIEW);
		fireEvent(AXE_EDITOR_EVENT_COPY_DATA);
		return;
	}

	if (isMouseInView())
	{
		dragging_start_pos = m_input.getMousePos();
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
	if (!isMouseInView())
		return;

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
	if (!isMouseInView())
		return;

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

bool MapEditor::save()
{
	return saveMap(map, "map-save.mdf", view);
}
bool MapEditor::load(std::string path)
{
	Map temp;
	if (!loadMap(temp, path, view))
	{
		std::cerr << "Failed to load map: " << path << std::endl;
		return false;
	}

	destroyMap(map);
	map = temp;

	undo_stack.clear();
	redo_stack.clear();

	if (!image_loaded)
	{
		enableKeybinds();
		image_loaded = true;
	}

	fireEvent(AXE_EDITOR_EVENT_COPY_DATA);
	// TODO: save viewer postion and set it here
	return true;
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
	if (zoom_out && view.scale > MIN_ZOOM)
	{
		View::scaleRelativeToPoint(view, m_input.getMousePos(), -ZOOM_FACTOR);
	}
	else if (!zoom_out && view.scale < MAX_ZOOM)
	{
		View::scaleRelativeToPoint(view, m_input.getMousePos(), ZOOM_FACTOR);
	}
}

bool MapEditor::isMouseInView()
{
	return m_input.getMousePos().isInBounds(view.screen_pos, view.screen_pos + view.size);
}

void MapEditor::fireEvent(int event_id)
{
	ALLEGRO_EVENT editor_event;
	editor_event.user.type = event_id;

	// For AXE_EDITOR_EVENT_MOVE_VIEW
	auto new_pos = screenToWorld(m_input.getMousePos(), view);

	switch (event_id)
	{
	case AXE_EDITOR_EVENT_COPY_DATA:
		editor_event.user.data1 = reinterpret_cast<intptr_t>(&map);
		break;

	case AXE_EDITOR_EVENT_MOVE_VIEW:
		editor_event.user.data1 = new_pos.x;
		editor_event.user.data2 = new_pos.y;
		break;

	case AXE_EDITOR_EVENT_ZOOM_IN: // Fall through
	case AXE_EDITOR_EVENT_ZOOM_OUT:
		break;

	case AXE_EDITOR_EVENT_SHOWHIDE_GRID:
		break;

	default:
		std::cerr << "Error: Invalid Event ID sent to MapEditor::fireEvent() - id: " << event_id << std::endl;
		exit(EXIT_FAILURE);
		return;
	}

	al_emit_user_event(&m_event_source, &editor_event, nullptr);
}

ALLEGRO_EVENT_SOURCE *MapEditor::getEventSource()
{
	return &m_event_source;
}

void MapEditor::enableKeybinds()
{
	m_input.setKeybind(MOUSE::WHEELUP, [this]()
					   { onMouseWheelUp(); });
	m_input.setKeybind(MOUSE::WHEELDOWN, [this]()
					   { onMouseWheelDown(); });
	m_input.setKeybind(MOUSE::MIDDLE, [this]()
					   { onMiddleMouseDown(); });
	m_input.setKeybind(
		MOUSE::MIDDLE, [this]()
		{ onMiddleMouseUp(); },
		false);
	m_input.setKeybind(MOUSE::LEFT, [this]()
					   { onLeftMouseDown(); });
	m_input.setKeybind(
		MOUSE::LEFT, [this]()
		{ onLeftMouseUp(); },
		false);
	m_input.setKeybind(MOUSE::RIGHT, [this]()
					   { onRightMouseDown(); });
	m_input.setKeybind(
		MOUSE::RIGHT, [this]()
		{ onRightMouseUp(); },
		false);
	m_input.setKeybind(ALLEGRO_KEY_G, [this]()
					   { if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL)) { fireEvent(AXE_EDITOR_EVENT_SHOWHIDE_GRID); return; } draw_grid = !draw_grid; });
	m_input.setKeybind(ALLEGRO_KEY_Z, [this]()
					   { undo(); });
	m_input.setKeybind(ALLEGRO_KEY_Y, [this]()
					   { redo(); });
	m_input.setKeybind(ALLEGRO_KEY_S, [this]()
					   { if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL)) save(); });
	m_input.setKeybind(ALLEGRO_KEY_L, [this]()
					   { if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL)) load("test.file"); }); // TODO: Need save to correct file and to expand fucntionality, save-as etc.
	m_input.setKeybind(ALLEGRO_KEY_C, [this]()
					   { if (m_input.isModifierDown(ALLEGRO_KEYMOD_CTRL)) view.world_pos = { 0, 0 }; });
	m_input.setKeybind(ALLEGRO_KEY_R, [this]()
					   { view.scale = 1.0; });
	m_input.setKeybind(ALLEGRO_KEY_SPACE, [this]()
					   { show_hidden = !show_hidden; });
	m_input.setKeybind(ALLEGRO_KEY_UP, [this]()
					   { fireEvent(AXE_EDITOR_EVENT_ZOOM_IN); });
	m_input.setKeybind(ALLEGRO_KEY_DOWN, [this]()
					   { fireEvent(AXE_EDITOR_EVENT_ZOOM_OUT); });
	m_input.setKeybind(ALLEGRO_KEY_U, [this]()
					   { fireEvent(AXE_EDITOR_EVENT_COPY_DATA); });
}

void MapEditor::disableKeybinds()
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
	m_input.clearKeybind(ALLEGRO_KEY_C);
	m_input.clearKeybind(ALLEGRO_KEY_R);
	m_input.clearKeybind(ALLEGRO_KEY_SPACE);
	m_input.clearKeybind(ALLEGRO_KEY_UP);
	m_input.clearKeybind(ALLEGRO_KEY_DOWN);
	m_input.clearKeybind(ALLEGRO_KEY_U);
}