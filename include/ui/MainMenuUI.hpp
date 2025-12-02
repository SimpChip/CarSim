
#ifndef CARSIM_MENUUI_HPP
#define CARSIM_MENUUI_HPP

#include "helpers/ResourceManager.hpp"
#include "controllers/Commands.hpp"

class MainMenuUI {
public:
    MainMenuUI() = default;
    void render(const GameCommands& cmd, const std::unique_ptr<ResourceManager>& rm, int currentAiCount);
};

#endif //CARSIM_MENUUI_HPP