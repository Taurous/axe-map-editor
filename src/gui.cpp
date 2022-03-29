#include "gui.hpp"
#include <iostream>

static bool show_window = false;
static ImFont *font_small = nullptr;
static ImFont *font_big = nullptr;

ImGuiIO& initGui(ALLEGRO_DISPLAY* d)
{
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->ClearFonts();
    font_small = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/iosevka-fixed-medium.ttf", 24);
    font_big = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/iosevka-fixed-medium.ttf", 40);

	ImGui::StyleColorsDark();
	ImGui_ImplAllegro5_Init(d);

    return io;
}

void renderGui(std::list<Creature>& creatures, ALLEGRO_EVENT_SOURCE *src)
{
    vec2i res = getScreenSize();
    float tracker_width = static_cast<float>(res.x) * 0.2;
    tracker_width = tracker_width < 300 ? 300 : tracker_width;
    ALLEGRO_EVENT gui_event;

    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();

    if (show_window) ImGui::ShowDemoWindow(&show_window);

// Display Main Menu Bar
    float main_menu_height = 0;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Show Demo Window")) show_window = true;
            if (ImGui::MenuItem("Close"))
            { 
                gui_event.user.type = AXE_GUI_EVENT_QUIT;
                al_emit_user_event(src, &gui_event, nullptr);
            };
            ImGui::EndMenu();
        }
        main_menu_height = ImGui::GetWindowHeight();
        ImGui::EndMainMenuBar();
    }

    renderInitiativeTracker({static_cast<float>(res.x) - tracker_width, main_menu_height},
        {tracker_width < 300.f ? 300.f : tracker_width,
        static_cast<float>(res.y) - main_menu_height - BOTTOM_BAR_HEIGHT}, creatures);
    
    ImGui::Render();

    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
}