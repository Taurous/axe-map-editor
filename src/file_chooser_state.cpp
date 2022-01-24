#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "file_chooser_state.hpp"

#include "input.hpp"
#include "state_machine.hpp"

FCState::FCState(StateMachine& state_machine, InputHandler& input) : AbstractState(state_machine, input)
{

}

FCState::~FCState() { };

void FCState::pause()
{
    m_input.clearKeybind(ALLEGRO_KEY_ESCAPE);
}
void FCState::resume()
{
    m_input.setKeybind(ALLEGRO_KEY_ESCAPE, [this](){ m_states.popState(); });
}

void FCState::handleEvents(const ALLEGRO_EVENT &ev)
{

}
void FCState::update(double delta_time)
{

}
void FCState::draw()
{
    al_draw_filled_rectangle(0, 0, 400, 400, al_map_rgb(255, 0, 255));
}