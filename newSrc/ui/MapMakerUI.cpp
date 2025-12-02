#include "ui/MapMakerUI.hpp"
#include "imgui.h"

void MapMakerUI::render(const GameCommands& cmd, const MapMakerSnap& snap) const {
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
