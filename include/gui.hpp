#pragma once

#include "imgui.h"
#include "imgui_impl_allegro5.h"
#include "util.hpp"

#include <string>

#include <allegro5/allegro.h>

enum DIALOG_TYPE
{
    NEW,
    LOAD
};

enum
{
    AXE_GUI_EVENT_ADD_CREATURE = ALLEGRO_GET_EVENT_TYPE('G','A','X','E'),
    AXE_GUI_EVENT_QUIT,
    AXE_GUI_EVENT_LOAD_MAP,
    AXE_GUI_EVENT_FILE_DIALOG_CREATE,
    AXE_GUI_EVENT_FILE_DIALOG_FINISHED
};

enum
{
    SIDE_WIDTH = 300,
    BOTTOM_BAR_HEIGHT = 40
};

class Gui
{
public:
    Gui(ALLEGRO_DISPLAY *display);
    ~Gui();

    void render();
    void setFileBufferText(std::string path);

    ALLEGRO_EVENT_SOURCE *getEventSource();

    bool captureInput();

private:
    ALLEGRO_DISPLAY *m_display;
    ALLEGRO_EVENT_SOURCE m_event_source;

    // Data
    bool m_show_demo_window;
    int m_tile_size;
    static char load_file_buffer[256];
};