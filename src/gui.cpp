#include "gui.hpp"
#include <iostream>

Gui::Gui(ALLEGRO_DISPLAY *display) : m_display(display), m_show_demo_window(false), m_tile_size(64)
{
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	ImGui_ImplAllegro5_Init(display);

    al_init_user_event_source(&m_event_source);
}

Gui::~Gui()
{
    ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();
}

ALLEGRO_EVENT_SOURCE *Gui::getEventSource()
{
    return &m_event_source;
}

bool Gui::captureInput()
{
    auto io = ImGui::GetIO();
    return io.WantCaptureKeyboard || io.WantCaptureMouse;
}

void Gui::render()
{
    bool show_file_dialog_modal = false;
    DIALOG_TYPE file_dialog_modal_type;
    (void)file_dialog_modal_type;
    
    vec2i res = getScreenSize();
    float tracker_width = static_cast<float>(res.x) * 0.2;
    tracker_width = tracker_width < 300 ? 300 : tracker_width;
    ALLEGRO_EVENT gui_event;

    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();

    if (m_show_demo_window) ImGui::ShowDemoWindow(&m_show_demo_window);

    // Main Menu
    float main_menu_height = 0;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Map"))
            {
                show_file_dialog_modal = true;
                file_dialog_modal_type = DIALOG_TYPE::NEW;
            }
            if (ImGui::MenuItem("Load Map"))
            {
                show_file_dialog_modal = true;
                file_dialog_modal_type = DIALOG_TYPE::LOAD;
            }
            if (ImGui::MenuItem("Show Demo Window")) m_show_demo_window = true;
            if (ImGui::MenuItem("Exit"))
            { 
                gui_event.user.type = AXE_GUI_EVENT_QUIT;
                al_emit_user_event(&m_event_source, &gui_event, nullptr);
            };
            ImGui::EndMenu();
        }
        main_menu_height = ImGui::GetWindowHeight();
        ImGui::EndMainMenuBar();
    }

    if (show_file_dialog_modal) ImGui::OpenPopup("file dialog");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("file dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        size_t buf_sz = 256;
        char buf[buf_sz] = {0};
        ImGui::Text("Choose image to load:");
        ImGui::InputTextWithHint("##Path", getHomeDir().c_str(), buf, buf_sz);
        ImGui::SameLine();
        if (ImGui::Button("..."))
        {
            // Open File Dialog
        }
        ImGui::InputInt("Tile Size", &m_tile_size);
        if (m_tile_size > 128) m_tile_size = 128;
        else if (m_tile_size < 8) m_tile_size = 8;

        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    /*gui_event.user.type = AXE_GUI_EVENT_FILE_DIALOG_CREATE;
    gui_event.user.data1 = DIALOG_TYPE::NEW;
    al_emit_user_event(src, &gui_event, nullptr);*/

    //Initiative Tracker
    ImGui::SetNextWindowSize(ImVec2(SIDE_WIDTH, res.y - BOTTOM_BAR_HEIGHT - main_menu_height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(res.x-SIDE_WIDTH, main_menu_height), ImGuiCond_Always);
    if (!ImGui::Begin("Initiative Tracker", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) ImGui::End();
    else
    {
        ImGui::BeginChildFrame(ImGui::GetID("Init"), ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiWindowFlags_NoBackground);
            //int id_count = 0;
            /*for (auto &c : creatures)
            {
                std::string id = c + std::to_string(id_count++);
                ImGui::BeginChild(ImGui::GetID(id.c_str()), ImVec2(0, 64), true);
                    ImGui::TextUnformatted(c.c_str());
                ImGui::EndChild();
            }*/
        ImGui::EndChildFrame();

        ImGui::Separator();

        if (ImGui::Button("Next"))
        {
            //std::string front = creatures.front();
            //creatures.pop_front();
            //creatures.push_back(front);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Creature"))
        {
            //creatures.push_back("Creature");
        }

        ImGui::End();
    }

    ImGui::Render();

    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
}