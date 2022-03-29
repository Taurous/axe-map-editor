#include "initiative_tracker.hpp"
#include "imgui.h"
#include <iostream>
#include <random>
#include <algorithm>

void TextCentered(std::string text) {
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::TextUnformatted(text.c_str());
}

std::string getStatAsString(int stat)
{
    assert(stat >= 0 && stat < MAX_STAT && "INVALID STAT AS STRING");
    switch (stat)
    {
        case STR: return "Str";
        case DEX: return "Dex";
        case CON: return "Con";
        case INT: return "Int";
        case WIS: return "Wis";
        case CHA: return "Cha";
        default: return "INVALID";
    }
}

void generateRandomCreatureList(std::list<Creature>& creatures, int count, bool append)
{
    assert(count >= 1 && "COUNT MUST BE GREATER THAN 0");

    std::vector<std::string> creature_names = {"Zombie", "Beholder", "Mimic", "Invisible Stalker", "Grung", "Strahd",
        "Deva", "Revenant", "Will-o-wisp", "Tarrasque", "Bodak"};
    std::default_random_engine gen;
    std::uniform_int_distribution<int> stat_dist(8, 18);
    std::uniform_int_distribution<int> name_dist(0, creature_names.size()-1);
    std::uniform_int_distribution<int> d20(1, 20);

    if (!append) creatures.clear();

    for (int i = 0; i < count; ++i)
    {
        Creature c;
        for (uint8_t s = 0; s < MAX_STAT; ++s) c.stats[s] = stat_dist(gen);

        c.initiative = d20(gen) + getStatModifier(c.stats[DEX]);
        c.name = creature_names[name_dist(gen)];

        creatures.push_back(c);
    }
}

void sortCreaturesByInitiative(std::list<Creature>& creatures)
{
    std::default_random_engine gen;
    std::uniform_int_distribution<int> dist(1, 20);
    creatures.sort([&gen, &dist](const Creature& a, const Creature& b){
        if (a.initiative == b.initiative)
        {
            int a_roll, b_roll = {0};
            while (a_roll == b_roll)
            {
                a_roll = dist(gen);
                b_roll = dist(gen);
            }

            return a_roll > b_roll;
        }
        return a.initiative > b.initiative;
    });
}

bool renderInitiativeTracker(const vec2f& pos, const vec2f& size, std::list<Creature>& creatures)
{
    ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Always);

    if (!ImGui::Begin("Initiative Tracker", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::End();
        return false;
    }

    ImGui::BeginChildFrame(ImGui::GetID("Initiative List"), ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiWindowFlags_NoBackground);
        int id_count = 0;
        for (auto &c : creatures)
        {
            std::string id = c.name + std::to_string(id_count++);
            ImGui::BeginChild(ImGui::GetID(id.c_str()), ImVec2(0, ImGui::GetFontSize()*5.5), true);
                TextCentered(c.name);

                for (int i = 0; i < MAX_STAT; ++i)
                {
                    ImGui::Text("%s: %2i", getStatAsString(i).c_str(), c.stats[i]);
                    if (i != CON && i != CHA) ImGui::SameLine();
                }

                ImGui::Text("Initiative: %2i", c.initiative);
            ImGui::EndChild();
        }
    ImGui::EndChildFrame();

    ImGui::Separator();

    if (ImGui::Button("Next"))
    {
        Creature front = creatures.front();
        creatures.pop_front();
        creatures.push_back(front);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Creature"))
    {
        if (!ImGui::IsPopupOpen("Add Creature"))
            ImGui::OpenPopup("Add Creature");
    }
    if (ImGui::BeginPopupModal("Add Creature", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char buf[64] = {0};
        ImGui::InputText("Name", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_AutoSelectAll);

        ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
        if (ImGui::Button("Add", button_size))
        {
            std::string n(buf);
            if (!n.empty())
            {
                Creature c;
                c.name = std::string(buf);
                creatures.push_back(c);
                ImGui::CloseCurrentPopup();
            }
            else
            {
                std::cout << "Creature name cannot be blank!\n";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", button_size))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::End();

    return true;
}