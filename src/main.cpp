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

#include "imgui.h"
#include "imgui_impl_allegro5.h"

#include "web.hpp"

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
constexpr int SIDE_WIDTH = 300;
constexpr int BOTTOM_BAR_HEIGHT = 40;

using std_clk = std::chrono::steady_clock;

void printHelp();
bool handleArgs(int argc, char** argv, std::string& path, int& tile_size);

int main(int argc, char** argv)
{
	//Testing dearimgui

	std::list<std::string> creatures = {"Carly", "Aksel", "Anthony", "Austin", "Chris", "Baelrog", "Stumpy", "Tarrasque", "Strahd", "Elephant", "Zombie", "Beholder"};

	//End Testing dearimgui

	if (((al_get_allegro_version() >> 8) & 255) < 7)
	{
		std::cerr << "Minimum Allegro5 version required is 5.2.7. You have " << getAllegroVersionStr() << " installed!\n";
		return -1;
	}

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

	// IMGUI

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();
	ImGui_ImplAllegro5_Init(display);
	bool show_window = false;

	// User Events

	ALLEGRO_EVENT_SOURCE editor_event_source;
	al_init_user_event_source(&editor_event_source);

	viewer_args.event_source = &editor_event_source;
	viewer_args.display_title = std::string(DISPLAY_TITLE) + " - Viewer";
	viewer_args.display_size = { DEFAULT_WIND_WIDTH, DEFAULT_WIND_HEIGHT };

	MapEditor map_editor(m_input, editor_event_source, viewer_args.image_path, viewer_args.tile_size, {0, 0}, { getScreenSize().x, getScreenSize().y-BOTTOM_BAR_HEIGHT });

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

		ImGui_ImplAllegro5_ProcessEvent(&ev);

		if (io.WantCaptureKeyboard || io.WantCaptureMouse)
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
			ImGui_ImplAllegro5_NewFrame();
			ImGui::NewFrame();

			if (show_window) ImGui::ShowDemoWindow(&show_window);

			float main_menu_height = 0;
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Show Demo Window")) show_window = true;
					if (ImGui::MenuItem("Close")) quit = true;
					ImGui::EndMenu();
				}
				main_menu_height = ImGui::GetWindowHeight();
				ImGui::EndMainMenuBar();
			}

			ImGui::SetNextWindowSize(ImVec2(SIDE_WIDTH, al_get_display_height(display) - BOTTOM_BAR_HEIGHT - main_menu_height), ImGuiCond_Always);
			ImGui::SetNextWindowPos(ImVec2(al_get_display_width(display)-SIDE_WIDTH, main_menu_height), ImGuiCond_Always);
			if (!ImGui::Begin("Initiative Tracker", nullptr,
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) ImGui::End();
			else
			{
				ImGui::BeginChildFrame(ImGui::GetID("Init"), ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiWindowFlags_NoBackground);
					int id_count = 0;
					for (auto &c : creatures)
					{
						std::string id = c + std::to_string(id_count++);
						ImGui::BeginChild(ImGui::GetID(id.c_str()), ImVec2(0, 64), true);
							ImGui::TextUnformatted(c.c_str());
						ImGui::EndChild();
					}
				ImGui::EndChildFrame();

				ImGui::Separator();

				if (ImGui::Button("Next"))
				{
					std::string front = creatures.front();
					creatures.pop_front();
					creatures.push_back(front);
				}
				ImGui::SameLine();
				if (ImGui::Button("Add Creature"))
				{
					creatures.push_back("Creature");
				}

				ImGui::End();
			}

			ImGui::Render();
			al_clear_to_color(al_map_rgb(0, 0, 0));
			map_editor.draw();
			ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
			al_flip_display();

			redraw = false;
		}
	}

	if (viewer_thread) al_set_thread_should_stop(viewer_thread);

	//Cleanup Imgui
	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();

	//Cleanup Allegro5
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
		<< "\t<tile-size> must be between " << MIN_TILE_SIZE << " and " << MAX_TILE_SIZE
		<< "\n\n-h, --help\t\tshow help" << std::endl;
}

bool handleArgs(int argc, char** argv, std::string& path, int& tile_size)
{
	if (argc < 2)
	{
		printHelp();
		return false;
	}
    
	if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")
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