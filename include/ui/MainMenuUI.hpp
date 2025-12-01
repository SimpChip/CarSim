
#ifndef CARSIM_MENUUI_HPP
#define CARSIM_MENUUI_HPP

#include "imgui.h"
#include "core/ResourceManager.hpp"
#include "controllers/Commands.hpp"

class MainMenuUI {
public:

    void render(const GameCommands& cmd, const std::unique_ptr<ResourceManager>& resources, const int currentAiCount) const {
        ImGui::Begin("Main Menu");

        const auto mapNames = resources->getMapNames();
        static int currentMap = 0;
        if (!mapNames.empty()) {
            std::vector<const char*> mapCStr;
            for (auto& m : mapNames) mapCStr.push_back(m.c_str());
            ImGui::Combo("Select Map", &currentMap, mapCStr.data(), mapCStr.size());
        }

        const auto modelNames = resources->getModelNames();
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
};

#endif //CARSIM_MENUUI_HPP