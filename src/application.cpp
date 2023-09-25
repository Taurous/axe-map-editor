#include "application.hpp"

#include <iostream> // For std::cout and std::Cerr
#include <chrono>	// To calculate delta time between ticks

#include <allegro5/allegro_primitives.h>

#include "util.hpp"

std::string getAllegroVersionStr();
ALLEGRO_DISPLAY *createDisplay(std::string title, int width, int height, int flags);

Application::Application() : display(nullptr), ev_queue(nullptr), timer(nullptr)
{

}

Application::~Application()
{
    
}

bool Application::init(std::vector<std::string> args, std::string display_title, const int display_width, const int display_height)
{
	double timer_speed = 1.0 / 60.0;

	// Check Allegro Version
	// Allegro 5.2.7 introduced keyboard modifiers being updated on
	// AL_KEY_DOWN/UP events which this app depends on.
    if (((al_get_allegro_version() >> 8) & 255) < 7)
	{
		std::cerr << "Minimum Allegro5 version required is 5.2.7. You have " << getAllegroVersionStr() << " installed!\n";
		return false;
	}

    if (!al_init())
	{
		std::cerr << "Failed to load Allegro!" << "\nAllegro Version: " << getAllegroVersionStr() << std::endl;
		return false;
	}

	display = createDisplay(display_title, display_width, display_height, ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);

	if (!display)
	{
		std::cerr << "Failed to create display!" << std::endl;
		return false;
	}

	input = new InputHandler();

	al_init_primitives_addon();

	timer = al_create_timer(timer_speed);
	ev_queue = al_create_event_queue();

	al_register_event_source(ev_queue, al_get_keyboard_event_source());
	al_register_event_source(ev_queue, al_get_mouse_event_source());
	al_register_event_source(ev_queue, al_get_timer_event_source(timer));
	al_register_event_source(ev_queue, al_get_display_event_source(display));

	editor = new MapEditor(input, {0, 0}, {display_width, display_height});

	return true;
}

bool Application::run()
{
	using std_clk = std::chrono::steady_clock;

	bool test_val = false;

	bool redraw = true;
	bool quit = false;
	std_clk::time_point current_time;
	double delta_time;

	// Set program lifetime keybinds
	input->setKeybind(ALLEGRO_KEY_ESCAPE, [&quit](){ quit = true; });

	al_start_timer(timer);
	auto last_time = std_clk::now();
	while (!quit)
	{
		al_wait_for_event(ev_queue, &ev);

		input->getInput(ev);
		editor->handleEvents(ev);

		switch (ev.type)
		{
			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				al_acknowledge_resize(ev.display.source);

				editor->resizeView({0, 0}, { al_get_display_width(display), al_get_display_height(display)});
			break;

			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				quit = true;
			break;

			case ALLEGRO_EVENT_TIMER:
				current_time = std_clk::now();
				delta_time = std::chrono::duration<double>(current_time - last_time).count();

				editor->update(delta_time);

				last_time = current_time;
				redraw = true;
			break;

			default:
			break;
		}
		
		if (al_event_queue_is_empty(ev_queue) && redraw)
		{
			al_clear_to_color(al_map_rgb(0x61, 0x80, 0x94));

			if (test_val) al_draw_rectangle(100, 100, 200, 200, al_map_rgb(255,0,0), 4);

			editor->draw();

			al_flip_display();

			redraw = false;
		}
	}

    return true;
}

void Application::quit()
{
	delete input;
	if (display) al_destroy_display(display);
	if (timer) al_destroy_timer(timer);
	if (ev_queue) al_destroy_event_queue(ev_queue);
	if (display) al_destroy_display(display);
}

std::string getAllegroVersionStr()
{
	uint32_t version = al_get_allegro_version();
	int major = version >> 24;
	int minor = (version >> 16) & 255;
	int revision = (version >> 8) & 255;
	int release = version & 255;

	std::stringstream ss;

	ss << major << '.' << minor << '.' << revision << '[' << release << ']';
	return ss.str();
}

ALLEGRO_DISPLAY *createDisplay(std::string title, int width, int height, int flags)
{
	ALLEGRO_DISPLAY *d = nullptr;

	al_set_new_display_flags(flags);

	al_set_new_window_title(title.c_str());

	d = al_create_display(width, height);

	return d;
}