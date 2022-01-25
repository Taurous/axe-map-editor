#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include "abstract_state.hpp"

class FCState : public AbstractState
{
public:
	FCState(StateMachine& state_machine, InputHandler& input);

	~FCState() override;

	void pause() override;
	void resume() override;

	void handleEvents(const ALLEGRO_EVENT &ev) override;
	void update(double delta_time) override;
	void draw() override;
private:
	ALLEGRO_FILECHOOSER *fc;
};