#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <allegro5/allegro.h>

#include "vec.hpp"
#include "util.hpp"
#include "lerp.hpp"
#include "view.hpp"
#include "map.hpp"
#include "editor_events.hpp"

using std_clk = std::chrono::steady_clock;

struct ViewerArgs
{
	int tile_size;
	std::string image_path;
    vec2i display_size;
    std::string display_title;
	ALLEGRO_EVENT_SOURCE *event_source;
};

void *viewer_thread_func(ALLEGRO_THREAD* thr, void* arg)
{
	ALLEGRO_DISPLAY* 		display 		= nullptr;
	ALLEGRO_EVENT_QUEUE* 	evq 			= nullptr;
	ALLEGRO_TIMER* 			timer 			= nullptr;
	ALLEGRO_EVENT 			ev;
	std_clk::time_point 	current_time;
	double 					delta_time;
	bool 					running 		= true;
	bool 					redraw 			= true;

	ViewerArgs *args = (ViewerArgs*)arg;
	View::ViewPort view;
	view.size = args->display_size;
	view.screen_pos = { 0, 0 };
	view.scale = 1.0;
	view.world_pos = { 0, 0 };
	Map map;
	bool grid = true;

	bool lerping = false;
	constexpr double lerp_time = 1.5;
	double elapsed = 0.0;
	vec2d view_start;
	vec2d view_target;

	display = createDisplay(args->display_title.c_str(), args->display_size.x, args->display_size.y, ALLEGRO_RESIZABLE | ALLEGRO_WINDOWED);
	timer = al_create_timer(1.0 / 60.0);
	evq = al_create_event_queue();

	al_register_event_source(evq, al_get_timer_event_source(timer));
	al_register_event_source(evq, al_get_display_event_source(display));
	al_register_event_source(evq, args->event_source);

	if (!createMap(map, args->image_path, args->tile_size))
	{
		std::cerr << "Viewer failed to load bitmap!\n\tImage path: " << args->image_path << std::endl;
		if (display) al_destroy_display(display);
		if (timer) al_destroy_timer(timer);
		if (evq) al_destroy_event_queue(evq);
		return NULL;
	}

	al_start_timer(timer);
	auto last_time = std_clk::now();
	while (running)
	{
		if (al_get_thread_should_stop(thr))
		{
			break;
		}

		al_wait_for_event(evq, &ev);
	
		vec2d diff;
		Map *t_map;
		switch (ev.type)
		{
			case AXE_EDITOR_EVENT_SHOWHIDE_GRID:
				grid = !grid;
			break;

			case AXE_EDITOR_EVENT_COPY_DATA:
				t_map = reinterpret_cast<Map*>(ev.user.data1);
				map.v_tiles = t_map->v_tiles;
				t_map = nullptr; // DO NOT CACHE ev.user.data1!
			break;

			case AXE_EDITOR_EVENT_MOVE_VIEW:
				view_target.x = static_cast<double>(ev.user.data1);
				view_target.y = static_cast<double>(ev.user.data2);
				view_start = view.world_pos;
				lerping = true;
				elapsed = 0.0;
			break;

			case AXE_EDITOR_EVENT_ZOOM_IN:
				if (view.scale < 5.0) view.scale += 0.1;
			break;

			case AXE_EDITOR_EVENT_ZOOM_OUT:
				if (view.scale >= 0.2) view.scale -= 0.1;
			break;

			case ALLEGRO_EVENT_TIMER:
				current_time = std_clk::now();
				delta_time = std::chrono::duration<double>(current_time - last_time).count();
				last_time = current_time;
				redraw = true;

				if (lerping && elapsed <= lerp_time)
				{
					elapsed += delta_time;
					view.world_pos = vec_lerp(view_start, view_target, easeInAndOutQuart(elapsed / lerp_time));
				}
				else lerping = false;
			break;

			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				running = false;
			break;

			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				al_acknowledge_resize(display);
				view.size = { al_get_display_width(display), al_get_display_height(display) };
			break;

			default:
			break;
		}

		if (al_event_queue_is_empty(evq) && redraw)
		{
			al_clear_to_color(al_map_rgb(0, 0, 0));

			drawMap(map, view, grid, false);

			al_flip_display();
			redraw = false;
		}
	}

	clearMap(map);

	al_destroy_timer(timer);
	al_destroy_event_queue(evq);
	al_destroy_display(display);

	return NULL;
}