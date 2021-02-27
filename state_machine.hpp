#pragma once

#include <memory>
#include <vector>

#include "abstract_state.hpp"

#include <allegro5/allegro.h>

class StateMachine
{
public:
	StateMachine();

	StateMachine(const StateMachine &other) = delete;
	StateMachine &operator=(const StateMachine &other) = delete;

	StateMachine(StateMachine &&other) = delete;
	StateMachine &operator=(StateMachine &&other) = delete;

	~StateMachine();

	void changeState(std::unique_ptr<AbstractState> state);
	void pushState(std::unique_ptr<AbstractState> state);
	void popState();

	void removeDeadStates();

	void handleEvents(const ALLEGRO_EVENT &ev);
	void update(float delta_time);
	void draw(bool draw_all);

	bool running() const { return m_running; }
	void quit() { m_running = false; }

private:
	bool m_running;
	bool m_res;

	std::vector<std::unique_ptr<AbstractState>> m_states;
	std::vector<std::unique_ptr<AbstractState>> m_dead_states;
};