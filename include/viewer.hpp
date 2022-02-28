#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <allegro5/allegro.h>

#include "vec.hpp"
#include "util.hpp"
#include "view.hpp"
#include "editor_events.hpp"

using std_clk = std::chrono::steady_clock;

struct ThreadArgs
{
	std::string bmp_path;
    vec2i display_size;
    std::string display_title;

	ALLEGRO_MUTEX *mutex;
	ALLEGRO_COND *cond;
	ALLEGRO_EVENT_SOURCE *event_source;
};

static void *thread_func(ALLEGRO_THREAD* thr, void* arg)
{
	ALLEGRO_DISPLAY* display = nullptr;
	ALLEGRO_EVENT_QUEUE* evq = nullptr;
	ALLEGRO_TIMER* timer = nullptr;
	ALLEGRO_EVENT ev;
	std_clk::time_point current_time;
	double delta_time;
	bool running = true;
	bool redraw = true;

	ThreadArgs *thargs = (ThreadArgs*)arg;
	View view;
	bool lerping = false;
	constexpr double lerp_time = 1.5;
	double elapsed = 0.0;
	vec2d view_start;
	vec2d view_target;
	ALLEGRO_BITMAP* bmp;

	display = createDisplay(thargs->display_title.c_str(), thargs->display_size.x, thargs->display_size.y, ALLEGRO_RESIZABLE | ALLEGRO_WINDOWED);
	timer = al_create_timer(1.0 / 60.0);
	evq = al_create_event_queue();

	al_register_event_source(evq, al_get_timer_event_source(timer));
	al_register_event_source(evq, al_get_display_event_source(display));
	al_register_event_source(evq, thargs->event_source);

	bmp = al_load_bitmap(thargs->bmp_path.c_str());
	if (!bmp)
	{
		std::cerr << "Failed to load bitmap in thread" << std::endl;
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
		switch (ev.type)
		{
			case AXE_EDITOR_EVENT_MOVE_VIEW:
				view_target.x = static_cast<double>(ev.user.data1);
				view_target.y = static_cast<double>(ev.user.data2);
				view_start = view.world_pos;
				lerping = true;
				elapsed = 0.0;
			break;

			case AXE_EDITOR_EVENT_SCALE_VIEW:
				std::cout << "AXE_EDITOR_EVENT_SCALE_VIEW\n" << "\tev.user.data1 = " << ev.user.data1 << "\n";
				if (ev.user.data1 > 0)
				{
					view.scale += { 0.1, 0.1 };
				}
				else if (ev.user.data1 < 0)
				{
					view.scale -= { 0.1, 0.1 };
				}
			break;

			case ALLEGRO_EVENT_TIMER:
				current_time = std_clk::now();
				delta_time = std::chrono::duration<double>(current_time - last_time).count();
				last_time = current_time;
				elapsed += delta_time;
				redraw = true;

				if (lerping && elapsed <= lerp_time)
				{
					view.world_pos = vec_lerp(view_start, view_target, easeInAndOutQuart(elapsed / lerp_time));
				}
				else lerping = false;
			break;

			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				running = false;
			break;

			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				al_acknowledge_resize(display);
			break;

			default:
			break;
		}

		if (al_event_queue_is_empty(evq) && redraw)
		{
			al_clear_to_color(al_map_rgb(45, 45, 45));

			int w = al_get_display_width(display);
			int h = al_get_display_height(display);

			al_draw_bitmap_region(bmp, view.world_pos.x - (w/2),view.world_pos.y - (h/2), w, h, 0, 0, 0);

			al_flip_display();
			redraw = false;
		}
	}

	al_destroy_bitmap(bmp);
	al_destroy_timer(timer);
	al_destroy_event_queue(evq);
	al_destroy_display(display);

	return NULL;
}