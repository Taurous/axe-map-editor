#pragma once

#include <string>
#include <vector>
#include <allegro5/allegro.h>

#include "input.hpp"
#include "map_editor.hpp"

class Application
{
public:
    Application();
    ~Application();

    bool init(std::vector<std::string> args, std::string display_title, const int display_width, const int display_height);
    bool run();
    void quit();

private:
    ALLEGRO_DISPLAY*		display	;
	ALLEGRO_EVENT_QUEUE*	ev_queue;
	ALLEGRO_TIMER*			timer;
	ALLEGRO_EVENT			ev;

    InputHandler*           input;
    MapEditor*              editor;
};