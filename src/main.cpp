/*	LICENSE
	Axe DnD Map Viewer
    Copyright (C) 2022  Aksel Huff

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
#include <allegro5/allegro_native_dialog.h>

#include "gui.hpp"
#include "util.hpp"
#include "input.hpp"
#include "viewer.hpp"
#include "map_editor.hpp"
#include "editor_events.hpp"

constexpr int 	DEFAULT_WIND_WIDTH	= 1280;
constexpr int 	DEFAULT_WIND_HEIGHT	= 768;
constexpr char 	DISPLAY_TITLE[]		= "Axe DnD Map";

using std_clk = std::chrono::steady_clock;

int main()
{
	if (!al_init())
	{
		std::cerr << "Failed to load Allegro!" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Check Allegro Version
	// Allegro 5.2.7 introduced keyboard modifiers being updated on
	// AL_KEY_DOWN/UP events which this app depends on.
	if (((al_get_allegro_version() >> 8) & 255) < 7)
	{
		std::cerr << "Minimum Allegro5 version required is 5.2.7. You have " << getAllegroVersionStr() << " installed!\n";
		return -1;
	}

	ALLEGRO_DISPLAY*		display			= nullptr;
	ALLEGRO_EVENT_QUEUE*	ev_queue		= nullptr;
	ALLEGRO_TIMER*			timer			= nullptr;
	ALLEGRO_THREAD*			viewer_thread	= nullptr;
	ALLEGRO_EVENT ev;

	std_clk::time_point current_time;
	double delta_time;

	bool redraw = true;
	bool quit = false;

	display = createDisplay(std::string(DISPLAY_TITLE) + " - Editor", DEFAULT_WIND_WIDTH, DEFAULT_WIND_HEIGHT, ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
	
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	al_init_native_dialog_addon();

	timer = al_create_timer(1.0 / 60.0);
	ev_queue = al_create_event_queue();

	// Systems
	InputHandler m_input; // Installs keyboard and mouse
	Gui gui(display);
	MapEditor map_editor(m_input, {0, 0}, { getScreenSize().x, getScreenSize().y - BOTTOM_BAR_HEIGHT });

	al_register_event_source(ev_queue, al_get_keyboard_event_source());
	al_register_event_source(ev_queue, al_get_mouse_event_source());
	al_register_event_source(ev_queue, al_get_timer_event_source(timer));
	al_register_event_source(ev_queue, al_get_display_event_source(display));
	al_register_event_source(ev_queue, gui.getEventSource());
	al_register_event_source(ev_queue, map_editor.getEventSource());

	ViewerArgs viewer_args;
	viewer_args.event_source = map_editor.getEventSource();
	viewer_args.display_title = std::string(DISPLAY_TITLE) + " - Viewer";
	viewer_args.display_size = { DEFAULT_WIND_WIDTH, DEFAULT_WIND_HEIGHT };

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

		// Stop handling input if file dialog is open
		ImGui_ImplAllegro5_ProcessEvent(&ev);

		if (gui.captureInput())
		{
			m_input.releaseKeys();
		}
		else
		{
			m_input.getInput(ev);
			map_editor.handleEvents(ev);
		}

		switch (ev.type)
		{
			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				ImGui_ImplAllegro5_InvalidateDeviceObjects();
				al_acknowledge_resize(ev.display.source);
				ImGui_ImplAllegro5_CreateDeviceObjects();

				map_editor.resizeView({0, 0}, { al_get_display_width(display), al_get_display_height(display) - BOTTOM_BAR_HEIGHT});
			break;

			case AXE_GUI_EVENT_QUIT: // Fall through
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				quit = true;
			break;

			/*case AXE_GUI_EVENT_FILE_DIALOG_CREATE:
				if (!file_dialog_open)
				{
					file_dialog = spawn_file_dialog(display, &gui_event_source, getHomeDir() + "/Pictures/", static_cast<DIALOG_TYPE>(ev.user.data1));
				}
				file_dialog_open = true;
			break;

			case AXE_GUI_EVENT_FILE_DIALOG_FINISHED:
				if (file_dialog_open)
				{
					if (al_get_native_file_dialog_count(file_dialog->file_dialog) > 0)
					{
						std::string path = al_get_native_file_dialog_path(file_dialog->file_dialog, 0);
						switch (file_dialog->type)
						{
							case DIALOG_TYPE::NEW:
								map_editor.create(path, 32); // TODO: Get tile size. (Open modal, then in modal spawn the file dialog to enter path into text box)
							break;
							case DIALOG_TYPE::LOAD:
								//map_editor.load(path); // Will not work until properly implement saving.
							break;
							default:
							break;
						}
					}

					stop_file_dialog(file_dialog);
					file_dialog_open = false;
				}
			break;*/

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
			gui.render();
			al_flip_display();

			redraw = false;
		}
	}

	if (viewer_thread) al_set_thread_should_stop(viewer_thread);

	//Cleanup Allegro5
	al_destroy_timer(timer);
	al_destroy_event_queue(ev_queue);
	al_destroy_display(display);

	al_destroy_thread(viewer_thread);

	return 0;
}