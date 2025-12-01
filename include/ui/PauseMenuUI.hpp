
#ifndef CARSIM_PAUSEMENUUI_HPP
#define CARSIM_PAUSEMENUUI_HPP

#include <functional>

#include "imgui.h"


class PauseMenuUI {
public:

    void render(const GameCommands& cmd) const {
        ImGui::Begin("Pause Menu");
        ImGui::Text("Game Paused");
        ImGui::Separator();

        if (ImGui::Button("Resume")){
            cmd.resume();
        }
        if (ImGui::Button("Quit to Menu")) {
            cmd.startMenu();
        }

        ImGui::End();
    }
};

#endif //CARSIM_PAUSEMENUUI_HPP