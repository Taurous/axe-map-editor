#pragma once

#include <list>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "abstract_state.hpp"

#include "view.h"
#include "map.hpp"
#include "command.hpp"
#include "edit_commands.hpp"

#include "util.hpp"

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

private:
	ALLEGRO_FONT* fn;

	View view;
	Map map;

	vec2f last_pos;
	vec2f mouse_pos;

	vec2i fill_start_pos;
	bool filling;

	bool dragging;

	bool draw_grid;

	std::list<std::unique_ptr<Command>> redo_stack;
	std::list<std::unique_ptr<Command>> undo_stack;

	void pushCommand(std::unique_ptr<Command> c);
};