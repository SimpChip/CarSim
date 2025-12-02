#ifndef CARSIM_RACEHUD_HPP
#define CARSIM_RACEHUD_HPP

#include "types/gameSnapshots.hpp"
#include "controllers/Commands.hpp"
class RaceHUD {
public:
    RaceHUD() = default;
    void render(const GameCommands& cmd, const GameSnap& state);
};

#endif //CARSIM_RACEHUD_HPP