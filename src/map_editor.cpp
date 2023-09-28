#include <iostream>
#include <math.h>
#include <functional>
#include <allegro5/allegro_color.h>

#include "map_editor.hpp"
#include "util.hpp"

constexpr size_t UNDO_STACK_LIMIT = 50;
constexpr double MIN_ZOOM = 0.62;
constexpr double MAX_ZOOM = 2.19;
constexpr double ZOOM_FACTOR = 0.08;

//Returns false if user is not hovering over a vertex, fills ret_vert with vertex info when return true
bool getHoveredVert(vec2d& ret_vert, const vec2d& mouse_position, const Map& map, const View::ViewPort& view);

void MapEditor::resizeView(vec2i view_pos, vec2i view_size)
{
	view.size = view_size;
	view.screen_pos = view_pos;
}

MapEditor::MapEditor(InputHandler *input, vec2i view_pos, vec2i view_size)
	: m_input(input), drawing_line(false), dragging(false), draw_grid(true), hovering_vertex(false)
{
	view.world_pos = {0.0, 0.0};
	view.scale = 1.0;
	resizeView(view_pos, view_size);

	map.setTile({ 0, 0 }, "test");

	enableKeybinds();
}

MapEditor::~MapEditor()
{
}

void MapEditor::handleEvents(const ALLEGRO_EVENT &ev)
{
	if (dragging) view.world_pos = dragging_world_begin + vec2d(dragging_mouse_begin - m_input->getMousePos()) / view.scale;

	if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)
	{
		vec2d vertex;
		if (getHoveredVert(vertex, m_input->getMousePos(), map, view))
		{
			hovered_vertex = vertex;
			hovering_vertex = true;
		}
		else hovering_vertex = false;
	}
}

void MapEditor::update(double delta_time)
{
	if (!dragging)
	{
		vec2d direction{0, 0};
		double vel = 500.0;
		if (m_input->isKeyDown(ALLEGRO_KEY_LSHIFT))
			vel *= 2.5;

		if (m_input->isKeyDown(ALLEGRO_KEY_A))
			direction.x -= 1.0;
		if (m_input->isKeyDown(ALLEGRO_KEY_D))
			direction.x += 1.0;
		if (m_input->isKeyDown(ALLEGRO_KEY_W))
			direction.y -= 1.0;
		if ((m_input->isKeyDown(ALLEGRO_KEY_S) && !m_input->isModifierDown(ALLEGRO_KEYMOD_CTRL)))
			direction.y += 1.0;

		view.world_pos += normalize(direction) * vel * delta_time;
	}
}

void MapEditor::draw()
{
	map.draw(view, draw_grid);

	if (hovering_vertex)
	{
		View::drawFilledCircle(view, hovered_vertex, 4, al_map_rgb(255, 0 ,0));
		View::drawCircle(view, hovered_vertex, 0.25 * map.getTileSz(), al_map_rgb(255, 0 ,0), 1);
	}
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
	zoomToCursor(false);
}
void MapEditor::onMouseWheelDown()
{
	zoomToCursor(true);
}
void MapEditor::onMiddleMouseUp()
{
	dragging = false;
}
void MapEditor::onMiddleMouseDown()
{
	dragging_world_begin = view.world_pos;
	dragging_mouse_begin = m_input->getMousePos();
	dragging = true;
}
void MapEditor::onLeftMouseUp()
{
}
void MapEditor::onLeftMouseDown()
{
	pushCommand(std::make_unique<SetTileCommand>(map, map.getTilePos(view, m_input->getMousePos()), "test"));
}

void MapEditor::onRightMouseUp()
{
}
void MapEditor::onRightMouseDown()
{
	if (map.tileExists(map.getTilePos(view, m_input->getMousePos())))
		pushCommand(std::make_unique<DeleteTileCommand>(map, map.getTilePos(view, m_input->getMousePos())));
}

void MapEditor::undo()
{
	if (!undo_stack.empty() && m_input->isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		Command *c = undo_stack.back().release();
		undo_stack.pop_back();

		std::cout << "Undoing...\n";

		c->undo();

		redo_stack.push_back(std::unique_ptr<Command>(c));
	}
}

void MapEditor::redo()
{
	if (!redo_stack.empty() && m_input->isModifierDown(ALLEGRO_KEYMOD_CTRL))
	{
		Command *c = redo_stack.back().release();
		redo_stack.pop_back();

		std::cout << "Redoing...\n";

		c->redo();

		undo_stack.push_back(std::unique_ptr<Command>(c));
	}
}

void MapEditor::zoomToCursor(bool zoom_out)
{
	if (zoom_out && view.scale > MIN_ZOOM)
	{
		View::scaleRelativeToPoint(view, m_input->getMousePos(), -ZOOM_FACTOR);
	}
	else if (!zoom_out && view.scale < MAX_ZOOM)
	{
		View::scaleRelativeToPoint(view, m_input->getMousePos(), ZOOM_FACTOR);
	}
}

bool MapEditor::isMouseInView()
{
	return m_input->getMousePos().isInBounds(view.screen_pos, view.screen_pos + view.size);
}

void MapEditor::enableKeybinds()
{
	m_input->setKeybind(MOUSE::WHEELUP, [this]()
		{ onMouseWheelUp(); });

	m_input->setKeybind(MOUSE::WHEELDOWN, [this]()
		{ onMouseWheelDown(); });

	m_input->setKeybind(MOUSE::MIDDLE, [this]()
		{ onMiddleMouseDown(); });

	m_input->setKeybind(
		MOUSE::MIDDLE, [this]()
		{ onMiddleMouseUp(); },
		false);

	m_input->setKeybind(MOUSE::LEFT, [this]()
		{ onLeftMouseDown(); });

	m_input->setKeybind(
		MOUSE::LEFT, [this]()
		{ onLeftMouseUp(); },
		false);

	m_input->setKeybind(MOUSE::RIGHT, [this]()
		{ onRightMouseDown(); });

	m_input->setKeybind(
		MOUSE::RIGHT, [this]()
		{ onRightMouseUp(); },
		false);

	m_input->setKeybind(ALLEGRO_KEY_G, [this]()
		{ draw_grid = !draw_grid; });

	m_input->setKeybind(ALLEGRO_KEY_Z, [this]()
		{ undo(); });

	m_input->setKeybind(ALLEGRO_KEY_Y, [this]()
		{ redo(); });

	m_input->setKeybind(ALLEGRO_KEY_C, [this]()
		{ if (m_input->isModifierDown(ALLEGRO_KEYMOD_CTRL)) view.world_pos = { 0, 0 }; });

	m_input->setKeybind(ALLEGRO_KEY_R, [this]()
		{ view.scale = 1.0; view.world_pos = { 0, 0 }; });
}

void MapEditor::disableKeybinds()
{
	m_input->clearKeybind(MOUSE::WHEELUP);
	m_input->clearKeybind(MOUSE::WHEELDOWN);
	m_input->clearKeybind(MOUSE::MIDDLE);
	m_input->clearKeybind(MOUSE::LEFT);
	m_input->clearKeybind(MOUSE::RIGHT);
	m_input->clearKeybind(ALLEGRO_KEY_G);
	m_input->clearKeybind(ALLEGRO_KEY_Z);
	m_input->clearKeybind(ALLEGRO_KEY_Y);
	m_input->clearKeybind(ALLEGRO_KEY_C);
	m_input->clearKeybind(ALLEGRO_KEY_R);
}

bool getHoveredVert(vec2d& ret_vert, const vec2d& mouse_position, const Map& map, const View::ViewPort& view)
{
	bool hovering = false;
	int tile_size = map.getTileSz();

	vec2d mouse_world_pos = View::screenToWorld(mouse_position, view);
	vec2i hovered_tile = vec2i(mouse_world_pos) / tile_size;

	if (mouse_world_pos.x < 0) hovered_tile.x -= 1;
	if (mouse_world_pos.y < 0) hovered_tile.y -= 1;

	vec2d tile_offset = mouse_world_pos / double(tile_size);

	vec2d vertices[4] = {
							vec2d(hovered_tile.x + 0, hovered_tile.y + 0),
							vec2d(hovered_tile.x + 1, hovered_tile.y + 0),
							vec2d(hovered_tile.x + 0, hovered_tile.y + 1),
							vec2d(hovered_tile.x + 1, hovered_tile.y + 1)
						};

	double closest = 1.0;
	int vert_chosen = 0;
	for (int i = 0; i < 4; ++i)
	{
		double distsq = magSquared(tile_offset - vertices[i]);

		if (distsq < closest)
		{
			vert_chosen = i;
			closest = distsq;
		}
	}

	if (closest < 0.25 * 0.25)
	{
		ret_vert = vertices[vert_chosen] * tile_size;
		hovering = true;
	}

	return hovering;
}