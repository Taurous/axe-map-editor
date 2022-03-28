#include "gui.hpp"

static bool show_window = false;

ImGuiIO& initGui(ALLEGRO_DISPLAY* d)
{
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	ImGui_ImplAllegro5_Init(d);

    return ImGui::GetIO();
}

void renderGui(std::list<std::string>& creatures, ALLEGRO_EVENT_SOURCE *src)
{
    vec2i res = getScreenSize();
    ALLEGRO_EVENT gui_event;

    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();

    if (show_window) ImGui::ShowDemoWindow(&show_window);

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

    ImGui::SetNextWindowSize(ImVec2(SIDE_WIDTH, res.y - BOTTOM_BAR_HEIGHT - main_menu_height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(res.x-SIDE_WIDTH, main_menu_height), ImGuiCond_Always);
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

    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
}