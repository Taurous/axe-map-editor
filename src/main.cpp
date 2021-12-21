/*	LICENSE
	DnD Battle Map
    Copyright (C) 2021  Aksel Huff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <iostream> // For std::cerr
#include <chrono>	// For FPS counting and providing game with tick time
#include <fstream>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "input.hpp"
#include "state_machine.hpp"
#include "editor_state.hpp"

constexpr int DEFAULT_WIND_WIDTH	= 1400;
constexpr int DEFAULT_WIND_HEIGHT	= 900;
std::string   DISPLAY_TITLE			= "Battle Maps";

int main(int argc, char ** argv)
{
	ALLEGRO_DISPLAY		*display	= nullptr;
	ALLEGRO_EVENT_QUEUE *ev_queue	= nullptr;
	ALLEGRO_TIMER		*timer		= nullptr;

#ifndef _DEBUG
	std::ofstream log("errorlog.txt");
	if (log.is_open())
	{
		std::cerr.rdbuf(log.rdbuf());
	}
#endif

	if (!al_init())
	{
		std::cerr << "Failed to load Allegro!" << std::endl;
		exit(EXIT_FAILURE);
	}

	al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
	al_set_new_window_title(DISPLAY_TITLE.c_str());
	display = al_create_display(DEFAULT_WIND_WIDTH, DEFAULT_WIND_HEIGHT);

	if (!display)
	{
		std::cerr << "Failed to create display!" << std::endl;
		exit(EXIT_FAILURE);
	}

	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();

	timer = al_create_timer(1.f / 60.f);
	ev_queue = al_create_event_queue();

	InputHandler m_input; // Installs keyboard and mouse

	al_register_event_source(ev_queue, al_get_keyboard_event_source());
	al_register_event_source(ev_queue, al_get_mouse_event_source());
	al_register_event_source(ev_queue, al_get_timer_event_source(timer));
	al_register_event_source(ev_queue, al_get_display_event_source(display));

	StateMachine m_sm;
	m_sm.pushState(std::make_unique<EditorState>(m_sm, m_input));

	bool redraw = true;
	al_start_timer(timer);
	auto last_time = std::chrono::steady_clock::now();
	while (m_sm.running())
	{
		ALLEGRO_EVENT ev;
		std::chrono::steady_clock::time_point current_time;
		float delta_time;

		al_wait_for_event(ev_queue, &ev);

		if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) al_acknowledge_resize(display);

		m_input.getInput(ev);
		m_sm.handleEvents(ev);

		if (m_input.isKeyPressed(ALLEGRO_KEY_ESCAPE)) m_sm.quit();

		switch (ev.type)
		{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			//TODO: Signal to state that window is being closed, prompt to save?
			m_sm.quit();
			break;

		case ALLEGRO_EVENT_TIMER:
			current_time = std::chrono::steady_clock::now();
			delta_time = std::chrono::duration<float>(current_time - last_time).count();
			last_time = current_time;
			m_sm.update(delta_time);
			m_sm.removeDeadStates();
			redraw = !redraw;
			break;

		default:
			break;
		}

		//Drawing

		if (al_event_queue_is_empty(ev_queue) && redraw)
		{
			al_clear_to_color(al_map_rgb(0, 0, 0));

			m_sm.draw(false);

			al_flip_display();
			redraw = false;
		}
	}

	al_destroy_timer(timer);
	al_destroy_event_queue(ev_queue);
	al_destroy_display(display);

	return 0;
}