//
// Created by Mella on 27.11.2025.
//

#ifndef CARSIM_RACEHUD_HPP
#define CARSIM_RACEHUD_HPP

#include "types/SnapShots.hpp"
#include "imgui.h"
#include "controllers/Commands.hpp"

class RaceHUD {
public:
    static void render(const GameCommands& cmd, const GameSnapShot& sta) {
        const auto lapMan = sta.lapManagerData;
        if (lapMan.laptrackers.empty()) {

            return;
        }
        auto formatTime = [](const float seconds) {
            const int totalMillis = static_cast<int>(seconds * 1000);
            const int minutes = totalMillis / 60000;
            const int secs    = (totalMillis / 1000) % 60;
            const int millis  = totalMillis % 1000;
            std::ostringstream oss;
            oss << std::setfill('0') << std::setw(2) << minutes << ":"
                << std::setw(2) << secs << "." << std::setw(3) << millis;
            return oss.str();
        };

        const auto playerCar = lapMan.laptrackers.at(sta.playerCarId);

        ImGui::Begin("Race HUD", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowPos(ImVec2(15, 15));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.f, 255));
        ImGui::SetWindowFontScale(20.f / ImGui::GetFontSize());
        ImGui::Text("Current Lap Time: %s", formatTime(lapMan.raceTimer - playerCar.lapStartTime).c_str());
        if (lapMan.raceTimer > 0) {
            ImGui::Text("Last Lap: %s", formatTime(playerCar.lastLapTime).c_str());
        }
        ImGui::Separator();
        ImGui::Text("Checkpoint: %d / %d", playerCar.currentCheckpoint, lapMan.checkpoints.size());

        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
    }
};

#endif //CARSIM_RACEHUD_HPP