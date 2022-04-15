#include "gui.hpp"
#include <iostream>

char Gui::load_file_buffer[256] = {0};

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

void Gui::setFileBufferText(std::string path)
{
    strcpy(Gui::load_file_buffer, path.c_str());
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
    state = GUI_STATE::NORMAL;
    
    vec2i res = getScreenSize();
    float tracker_width = static_cast<float>(res.x) * 0.2;
    tracker_width = tracker_width < 300 ? 300 : tracker_width;
    ALLEGRO_EVENT gui_event;

    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();

    if (m_show_demo_window) ImGui::ShowDemoWindow(&m_show_demo_window);

    // Main Menu
    float main_menu_height = renderMainMenu();

    switch (state)
    {
        case GUI_STATE::CREATE_POPUP:
            ImGui::OpenPopup("Create Map");
        break;
        case GUI_STATE::LOAD_POPUP:
            //ImGui::OpenPopup("Load Map");
        break;
        case GUI_STATE::SAVE_POPUP:
            //ImGui::OpenPopup("Save Map");
        break;
        default:
        break;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Create Map", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Choose image to load:");
        ImGui::InputText("##Path", Gui::load_file_buffer, IM_ARRAYSIZE(Gui::load_file_buffer));
        ImGui::SameLine();
        if (ImGui::Button("..."))
        {
           gui_event.user.type = AXE_GUI_EVENT_FILE_DIALOG_CREATE;
           gui_event.user.data1 = static_cast<DIALOG_TYPE>(DIALOG_TYPE::NEW);
           al_emit_user_event(&m_event_source, &gui_event, nullptr);
        }
        ImGui::InputInt("Tile Size", &m_tile_size);
        if (m_tile_size > 128) m_tile_size = 128;
        else if (m_tile_size < 8) m_tile_size = 8;

        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            gui_event.user.type = AXE_GUI_EVENT_NEW_MAP;
            gui_event.user.data1 = static_cast<DIALOG_TYPE>(DIALOG_TYPE::NEW);
            gui_event.user.data2 = m_tile_size;
           al_emit_user_event(&m_event_source, &gui_event, nullptr);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    renderInitiativeTracker(main_menu_height);

    ImGui::Render();

    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
}

int Gui::renderMainMenu()
{
    int height = 0;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Map"))
            {
                memset(Gui::load_file_buffer, 0, sizeof(Gui::load_file_buffer));
                state = GUI_STATE::CREATE_POPUP;
            }
            if (ImGui::MenuItem("Load Map"))
            {
                memset(Gui::load_file_buffer, 0, sizeof(Gui::load_file_buffer));
                state = GUI_STATE::LOAD_POPUP;
            }
            if (ImGui::MenuItem("Show Demo Window")) m_show_demo_window = true;
            if (ImGui::MenuItem("Exit"))
            { 
                ALLEGRO_EVENT ev;
                ev.user.type = AXE_GUI_EVENT_QUIT;
                al_emit_user_event(&m_event_source, &ev, nullptr);
            };
            ImGui::EndMenu();
        }
        height = ImGui::GetWindowHeight();
        ImGui::EndMainMenuBar();
    }

    return height;
}

void Gui::renderInitiativeTracker(int menu_height)
{
    ImGui::SetNextWindowSize(ImVec2(SIDE_WIDTH, getScreenSize().y - BOTTOM_BAR_HEIGHT - menu_height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(getScreenSize().x-SIDE_WIDTH, menu_height), ImGuiCond_Always);
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
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Creature"))
        {
        }

        ImGui::End();
    }
}