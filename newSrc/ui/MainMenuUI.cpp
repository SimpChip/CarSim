#include "ui/MainMenuUi.hpp"
#include "imgui.h"

void MainMenuUI::render(const GameCommands& cmd, const std::unique_ptr<ResourceManager>& rm, const int currentAiCount) {
    ImGui::Begin("Main Menu");

    const auto mapNames = rm->getMapNames();
    static int currentMap = 0;
    if (!mapNames.empty()) {
        std::vector<const char*> mapCStr;
        for (auto& m : mapNames) mapCStr.push_back(m.c_str());
        ImGui::Combo("Select Map", &currentMap, mapCStr.data(), mapCStr.size());
    }

    const auto modelNames = rm->getModelNames();
    static int currentModel = 0;
    if (!modelNames.empty()) {
        std::vector<const char*> modelCStr;
        for (auto& m : modelNames) modelCStr.push_back(m.c_str());
        ImGui::Combo("Select Model", &currentModel, modelCStr.data(), modelCStr.size());
    }
    static int newAiCount = currentAiCount;
    ImGui::SliderInt("Bots", &newAiCount, 0, 120);

    if (ImGui::Button("Start Race")) {
        cmd.startRace(mapNames[currentMap], newAiCount, modelNames[currentModel]);
    }
    if (ImGui::Button("Start Map Maker")) {
        cmd.startMapMaker();
    }
    ImGui::End();
}