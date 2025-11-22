//
// Created by Mella on 13.11.2025.
//

#ifndef CARSIM_RACEHUD_HPP
#define CARSIM_RACEHUD_HPP

#include <imgui.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <GameLogic.hpp>

class RaceHUD {
    float finishPopUpTimer = 0.f;

public:
    void update(const float dt) {
        if (finishPopUpTimer > 0.f) {
            finishPopUpTimer -= dt;
        }
    }

    void render(GameLogic& gameLogic) {
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

        ImGui::Begin("Race HUD", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowPos(ImVec2(15, 15));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.f, 255));
        ImGui::SetWindowFontScale(20.f / ImGui::GetFontSize());
        ImGui::Text("Current Lap Time: %s", formatTime(gameLogic.getCurrentLapTime()).c_str());
        if (gameLogic.getLastLapTime() > 0) {
            ImGui::Text("Last Lap: %s", formatTime(gameLogic.getLastLapTime()).c_str());
            finishPopUpTimer = 3;
        }
        ImGui::Separator();
        if (gameLogic.getCpLogic().allCheckpointsPassed()) {
            ImGui::Text("Checkpoint: ALL PASSED");
        } else {
            ImGui::Text("Checkpoint: %d / %d", gameLogic.getCpLogic().getCurrentCpIndex(), gameLogic.getCpLogic().getCheckpoints().size());
        }
        if (finishPopUpTimer > 0) {
            ImGui::Text("FINISHED A LAP?!??!?");
        }
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
    }
};
#endif //CARSIM_RACEHUD_HPP