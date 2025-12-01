
#ifndef CARSIM_COMMANDS_HPP
#define CARSIM_COMMANDS_HPP

#include <functional>
#include <string>

struct GameCommands {
    std::function<void(const std::string& map, int aiCount, const std::string& model)> startRace;
    std::function<void()> startMapMaker;
    std::function<void()> pause;
    std::function<void()> resume;
    std::function<void()> startMenu;
};

#endif //CARSIM_COMMANDS_HPP