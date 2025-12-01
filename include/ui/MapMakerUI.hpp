#ifndef CARSIM_MAPMAKERUI_HPP
#define CARSIM_MAPMAKERUI_HPP

#include "imgui.h"
#include "types/SnapShots.hpp"
#include "controllers/Commands.hpp"

class MapMakerUI {
public:

    std::function<void()> onSave;
    std::function<void()> onUndo;
    std::function<void()> onClear;
    std::function<void()> onGoBack;
    std::function<void(bool)> onLoopedChanged;
    std::function<void(int)> onMapSizeChanged;

    void render(const GameCommands& cmd, const MapMakerSnapshot& snap) const {
        ImGui::Begin("Map Maker", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);

        if (ImGui::Button("Save") && onSave) onSave();
        if (ImGui::Button("Undo Last") && onUndo) onUndo();
        if (ImGui::Button("Clear All") && onClear) onClear();
        if (ImGui::Button("Go Back To Menu") && onGoBack) onGoBack();

        bool loopedState = snap.looped;
        if (ImGui::Checkbox("Looped", &loopedState) && onLoopedChanged) {
            onLoopedChanged(loopedState);
        }
        static int newSize = snap.mapSize;
        if (ImGui::SliderInt("Map Size", &newSize, 50, 10000) && onMapSizeChanged) {
            onMapSizeChanged(newSize);
        }

        ImGui::Separator();
        for (const auto& point : snap.points) {
            ImGui::Text("X: %.2f , Y: %.2f , Z: %.2f", point.x, point.y, point.z);
        }

        ImGui::End();
    }
};

#endif //CARSIM_MAPMAKERUI_HPP