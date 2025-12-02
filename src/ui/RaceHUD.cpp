#include "ui/RaceHUD.hpp"

#include <iomanip>

#include "imgui.h"

void RaceHUD::render(const GameCommands& cmd, const GameSnap& state) {
    const auto& lap = state.lap;
    const auto& laptrackers = lap.laptrackers;
    if (laptrackers.empty()) {

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

    const auto playerCar = laptrackers.at(state.playerCarId);

    ImGui::Begin("Race HUD", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
    ImGui::SetWindowPos(ImVec2(15, 15));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.f, 255));
    ImGui::SetWindowFontScale(20.f / ImGui::GetFontSize());
    ImGui::Text("Current Lap Time: %s", formatTime(lap.raceTimer - playerCar.lapStartTime).c_str());
    if (lap.raceTimer > 0) {
        ImGui::Text("Last Lap: %s", formatTime(playerCar.lastLapTime).c_str());
    }
    ImGui::Separator();
    ImGui::Text("Checkpoint: %d / %d", playerCar.currentCheckpoint, lap.checkpoints.size());

    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}