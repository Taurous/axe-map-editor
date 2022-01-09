#pragma once

#include <list>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "abstract_state.hpp"

#include "view.hpp"
#include "map.hpp"
#include "tile_selector.hpp"
#include "edit_commands.hpp"

enum PLACER
{
	TILE,
	WALL,
	OBJECT
};

class EditorState : public AbstractState
{
public:
	EditorState(StateMachine& state_machine, InputHandler& input);

	~EditorState() override;

	void pause() override;
	void resume() override;

	void handleEvents(const ALLEGRO_EVENT &ev) override;
	void update(float delta_time) override;
	void draw() override;

	void saveMap();
	void loadMap();

	void onMouseWheelUp();
	void onMouseWheelDown();
	void onMiddleMouseUp();
	void onMiddleMouseDown();
	void onLeftMouseUp();
	void onLeftMouseDown();
	void onRightMouseDown();
	void onRightMouseUp();

private:
	ALLEGRO_FONT* fn;

	View view;
	Map map;
	TileSelector ts;

	vec2f last_pos;
	vec2f mouse_pos;

	vec2i fill_start_pos;
	bool dragging;
	bool filling;
	bool draw_grid;

	std::list<std::unique_ptr<Command>> redo_stack;
	std::list<std::unique_ptr<Command>> undo_stack;

	void pushCommand(std::unique_ptr<Command> c);
};