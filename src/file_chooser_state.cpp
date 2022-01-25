#include <iostream>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "file_chooser_state.hpp"

#include "input.hpp"
#include "state_machine.hpp"

FCState::FCState(StateMachine& state_machine, InputHandler& input) : AbstractState(state_machine, input), fc(nullptr)
{
    fc = al_create_native_file_dialog(NULL, "Choose Map Image", "*.png;*.jpg;*.jpeg", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
    if (al_show_native_file_dialog(NULL, fc))
    {
        int num_files = al_get_native_file_dialog_count(fc);

        for (int i = 0; i < num_files; ++i)
        {
            std::cout << al_get_native_file_dialog_path(fc, i) << "\n";
        }

        al_destroy_native_file_dialog(fc);
    }
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
    al_draw_filled_rectangle(0, 0, 200, 200, al_map_rgb(255,0,255));
}