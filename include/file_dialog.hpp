#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>

#include "gui.hpp"
#include "util.hpp"

struct AsyncDialog
{
    ALLEGRO_DISPLAY* display;
    ALLEGRO_FILECHOOSER* file_dialog;
    ALLEGRO_EVENT_SOURCE* evt_src;
    ALLEGRO_THREAD* thr;
    DIALOG_TYPE type;
};

static void* file_dialog_thread_func(ALLEGRO_THREAD* thr, void* args)
{
    AsyncDialog* data = reinterpret_cast<AsyncDialog*>(args);

    ALLEGRO_EVENT ev;
    (void)thr;

    al_show_native_file_dialog(data->display, data->file_dialog);

    ev.user.type = AXE_GUI_EVENT_FILE_DIALOG_FINISHED;
    ev.user.data1 = data->type;
    al_emit_user_event(data->evt_src, &ev, nullptr);

    return nullptr;
}

static void stop_file_dialog(AsyncDialog* data)
{
    if (data)
    {
        al_destroy_thread(data->thr);
        if (data->file_dialog) al_destroy_native_file_dialog(data->file_dialog);
    }
    delete data;
}

static AsyncDialog* spawn_file_dialog(ALLEGRO_DISPLAY* disp, ALLEGRO_EVENT_SOURCE* src, const std::string& initial_path, DIALOG_TYPE type)
{
    AsyncDialog *data = new AsyncDialog;

    switch (type)
    {
        case DIALOG_TYPE::NEW:
            data->file_dialog = al_create_native_file_dialog(initial_path.c_str(), "Choose Map Image", "*.jpg;*.jpeg;*.png;*.tga;*.tga", ALLEGRO_FILECHOOSER_PICTURES);
        break;
        case DIALOG_TYPE::LOAD:
            data->file_dialog = al_create_native_file_dialog(initial_path.c_str(), "Load Map", "*.mdf", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
        break;
        default:
            std::cerr << "Passed invalid type (" << type << ") to spawn_file_dialog()\n";
            delete data;
            return nullptr;
        break;
    }

    data->display = disp;
    data->evt_src = src;
    data->type = type;
    data->thr = al_create_thread(file_dialog_thread_func, data);
    al_start_thread(data->thr);

    return data;
}