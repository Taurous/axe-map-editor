/*	LICENSE
	Axe DnD Map Viewer
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

#include <iostream> // For std::cout and std::Cerr
#include <chrono>	// To calculate delta time between ticks

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "util.hpp"
#include "input.hpp"
#include "viewer.hpp"
#include "map_editor.hpp"
#include "editor_events.hpp"

constexpr int 	DEFAULT_WIND_WIDTH	= 1280;
constexpr int 	DEFAULT_WIND_HEIGHT	= 768;
constexpr char 	DISPLAY_TITLE[]		= "Axe DnD Map";

constexpr int MIN_TILE_SIZE = 16;
constexpr int MAX_TILE_SIZE = 128;

using std_clk = std::chrono::steady_clock;

void printHelp();
bool handleArgs(int argc, char** argv, std::string& path, int& tile_size);

int main(int argc, char** argv)
{
	// Get command line arguments
	ViewerArgs viewer_args; // Passed to viewer thread function, and map_editor constructor
	if (!handleArgs(argc, argv, viewer_args.image_path, viewer_args.tile_size)) return -1;

	ALLEGRO_DISPLAY*		display			= nullptr;
	ALLEGRO_EVENT_QUEUE*	ev_queue		= nullptr;
	ALLEGRO_TIMER*			timer			= nullptr;
	ALLEGRO_THREAD*			viewer_thread	= nullptr;
	ALLEGRO_EVENT ev;
	std_clk::time_point current_time;
	double delta_time;
	
	bool redraw = true;
	bool quit = false;

	if (!al_init())
	{
		std::cerr << "Failed to load Allegro!" << std::endl;
		exit(EXIT_FAILURE);
	}

	display = createDisplay(std::string(DISPLAY_TITLE) + " - Editor", DEFAULT_WIND_WIDTH, DEFAULT_WIND_HEIGHT, ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
	
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();

	timer = al_create_timer(1.0 / 60.0);
	ev_queue = al_create_event_queue();

	InputHandler m_input; // Installs keyboard and mouse

	al_register_event_source(ev_queue, al_get_keyboard_event_source());
	al_register_event_source(ev_queue, al_get_mouse_event_source());
	al_register_event_source(ev_queue, al_get_timer_event_source(timer));
	al_register_event_source(ev_queue, al_get_display_event_source(display));

	// User Events

	ALLEGRO_EVENT_SOURCE editor_event_source;
	al_init_user_event_source(&editor_event_source);

	viewer_args.event_source = &editor_event_source;
	viewer_args.display_title = std::string(DISPLAY_TITLE) + " - Viewer";
	viewer_args.display_size = { DEFAULT_WIND_WIDTH, DEFAULT_WIND_HEIGHT };

	MapEditor map_editor(m_input, editor_event_source, viewer_args.image_path, viewer_args.tile_size, {0, 0}, { getScreenSize().x, getScreenSize().y });

	// Set program lifetime keybinds
	m_input.setKeybind(ALLEGRO_KEY_ESCAPE, 	[&quit](){ quit = true; });
	m_input.setKeybind(ALLEGRO_KEY_F1,		[&](){
		al_destroy_thread(viewer_thread); // Safely returns null if viewer_thread passed in is null
		viewer_thread = nullptr;

		viewer_thread = al_create_thread(viewer_thread_func, &viewer_args);
		al_start_thread(viewer_thread);
		map_editor.fireEvent(AXE_EDITOR_EVENT_COPY_DATA);	
	});

	al_start_timer(timer);
	auto last_time = std_clk::now();
	while (!quit)
	{
		al_wait_for_event(ev_queue, &ev);

		// Skip any events where the focus is the view display
		if (ev.any.source == al_get_mouse_event_source())
		{
			if (ev.mouse.display != display) continue;
		}
		else if (ev.any.source == al_get_keyboard_event_source())
		{
			if (ev.keyboard.display != display) continue;
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

	if (viewer_thread) al_set_thread_should_stop(viewer_thread);

	al_destroy_timer(timer);
	al_destroy_event_queue(ev_queue);
	al_destroy_display(display);

	al_destroy_thread(viewer_thread);

	return 0;
}

void printHelp()
{
	std::cout << "Usage: axe-map-editor <path-to-image> <tile-size>\n"
		<< "\tSupported image types are jpg, png, tga, bmp\n"
		<< "\tTile-size must be between " << MIN_TILE_SIZE << " and " << MAX_TILE_SIZE
		<< "\n\t-h to see this screen again" << std::endl;
}

bool handleArgs(int argc, char** argv, std::string& path, int& tile_size)
{
	if (argc < 2)
	{
		printHelp();
		return false;
	}
    
	if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "-help")
	{
		printHelp();
		return false;
	}

	if (argc < 3)
	{
		std::cerr << "Too few arguments!\n";
		printHelp();
		return false;
	}

	path = argv[1];
	tile_size = atoi(argv[2]);

	if (tile_size < MIN_TILE_SIZE || tile_size > MAX_TILE_SIZE) // TODO: Remove magic #, also document smallest allowed tile size
	{
		std::cerr << "Invalid tile size!\n";
		return false;
	}

	return true;
}