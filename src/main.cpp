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
#include <allegro5/allegro_native_dialog.h>

#include "input.hpp"
#include "util.hpp"

#include "viewer.hpp"
#include "map_editor.hpp"

#include "editor_events.hpp"

constexpr int 	DEFAULT_WIND_WIDTH	= 1400;
constexpr int 	DEFAULT_WIND_HEIGHT	= 900;
constexpr char 	DISPLAY_TITLE[]		= "Axe DnD Map";

using std_clk = std::chrono::steady_clock;

int main(int argc, char ** argv)
{
	/*if (argc < 3)
	{
		std::cout << "Usage: axe-map-editor <path-to-map-image> <tile-size>" << std::endl;
		return -1;
	}*/

	Map in_map;
	in_map.path = "/home/aksel/Downloads/map_32x32(2).png";//argv[1];
	in_map.tile_size = 100;//atoi(argv[2]);

	ALLEGRO_DISPLAY*		main_display	= nullptr;
	ALLEGRO_EVENT_QUEUE*	ev_queue		= nullptr;
	ALLEGRO_TIMER*			timer			= nullptr;
	ALLEGRO_THREAD*			view_thread		= nullptr;
	ALLEGRO_EVENT ev;
	std_clk::time_point current_time;
	double delta_time;
	
	bool redraw = true;
	bool quit = false;

	ThreadArgs thargs;
	thargs.in_map = in_map;
	thargs.mutex = al_create_mutex();
	thargs.cond = al_create_cond();

	if (!al_init())
	{
		std::cerr << "Failed to load Allegro!" << std::endl;
		exit(EXIT_FAILURE);
	}

	main_display = createDisplay(std::string(DISPLAY_TITLE) + " - Editor", DEFAULT_WIND_WIDTH, DEFAULT_WIND_HEIGHT, ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);

	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	al_init_native_dialog_addon();

	timer = al_create_timer(1.0 / 60.0);
	ev_queue = al_create_event_queue();

	InputHandler m_input; // Installs keyboard and mouse

	al_register_event_source(ev_queue, al_get_keyboard_event_source());
	al_register_event_source(ev_queue, al_get_mouse_event_source());
	al_register_event_source(ev_queue, al_get_timer_event_source(timer));
	al_register_event_source(ev_queue, al_get_display_event_source(main_display));

	// User Events

	ALLEGRO_EVENT_SOURCE editor_event_source;
	al_init_user_event_source(&editor_event_source);

	thargs.event_source = &editor_event_source;
	thargs.display_title = std::string(DISPLAY_TITLE) + " - Viewer";
	thargs.display_size = { DEFAULT_WIND_WIDTH, DEFAULT_WIND_HEIGHT };

	MapEditor map_editor(in_map, m_input, editor_event_source, {0, 0}, { getScreenSize().x, getScreenSize().y });

	// Set program lifetime keybinds
	m_input.setKeybind(ALLEGRO_KEY_ESCAPE, 	[&quit](){ quit = true; });
	m_input.setKeybind(ALLEGRO_KEY_F1,		[&](){
		al_destroy_thread(view_thread);

		view_thread = nullptr;

		// Pass pointer to map data into thread

		view_thread = al_create_thread(thread_func, &thargs);
		al_start_thread(view_thread);
		map_editor.fireEvent(AXE_EDITOR_EVENT_COPY_DATA);	
	});

	m_input.callKeybind(ALLEGRO_KEY_F1);

	al_start_timer(timer);
	auto last_time = std_clk::now();
	while (!quit)
	{
		al_wait_for_event(ev_queue, &ev);

		// Skip any events where the focus is the view display
		if (ev.any.source == al_get_mouse_event_source())
		{
			if (ev.mouse.display != main_display)
			{
				continue;
			}
		}
		else if (ev.any.source == al_get_keyboard_event_source())
		{
			if (ev.keyboard.display != main_display)
			{
				continue;
			}
		}

		m_input.getInput(ev);
		map_editor.handleEvents(ev);
	
		switch (ev.type)
		{
			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				al_acknowledge_resize(ev.display.source);
				map_editor.resizeView({0, 0}, { getScreenSize().x, getScreenSize().y });
			break;

			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				quit = true;
			break;

			case ALLEGRO_EVENT_TIMER:
				current_time = std_clk::now();
				delta_time = std::chrono::duration<double>(current_time - last_time).count();
				last_time = current_time;
				map_editor.update(delta_time);
				redraw = true;
			break;

			default:
			break;
		}
		
		//Drawing

		if (al_event_queue_is_empty(ev_queue) && redraw)
		{
			al_clear_to_color(al_map_rgb(0, 0, 0));

			map_editor.draw();

			al_flip_display();

			redraw = false;
		}
	}

	if (view_thread) al_set_thread_should_stop(view_thread);

	al_destroy_timer(timer);
	al_destroy_event_queue(ev_queue);
	al_destroy_display(main_display);

	if (thargs.mutex) al_destroy_mutex(thargs.mutex);
	if (thargs.cond) al_destroy_cond(thargs.cond);
	al_destroy_thread(view_thread);

	return 0;
}