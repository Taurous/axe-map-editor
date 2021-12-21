#pragma once

#include <allegro5/allegro.h>

class StateMachine;
class InputHandler;

class AbstractState
{
public:
	AbstractState(StateMachine &state_machine, InputHandler &input)
		: m_states(state_machine), m_input(input)
	{ }

	virtual ~AbstractState() { }

	virtual void pause() = 0;
	virtual void resume() = 0;

	virtual void handleEvents(const ALLEGRO_EVENT &ev) = 0;
	virtual void update(float delta_time) = 0;
	virtual void draw() = 0;

protected:
	StateMachine &m_states;
	InputHandler &m_input;
};