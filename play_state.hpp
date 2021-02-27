#pragma once
#include "abstract_state.hpp"

#include <iostream> // For Testing
#include <bitset>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "vec.hpp"
#include "map.hpp"

class PlayState : public AbstractState
{
public:
	PlayState(StateMachine& state_machine, InputHandler& input);

	~PlayState() override;

	void pause() override;
	void resume() override;

	void handleEvents() override;
	void update(float delta_time) override;
	void draw() override;

private:
	std::unique_ptr<Map> map;

	ALLEGRO_FONT* _debug_font;
};