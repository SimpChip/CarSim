#ifndef CARSIM_MAPMAKERUI_HPP
#define CARSIM_MAPMAKERUI_HPP

#include <functional>
#include "controllers/Commands.hpp"
#include "types/mapMakerSnapshots.hpp"

class MapMakerUI {
public:
    std::function<void()> onSave;
    std::function<void()> onUndo;
    std::function<void()> onClear;
    std::function<void()> onGoBack;
    std::function<void(bool)> onLoopedChanged;
    std::function<void(int)> onMapSizeChanged;

    MapMakerUI() = default;
    void render(const GameCommands& cmd, const MapMakerSnap& snap) const;
};

#endif //CARSIM_MAPMAKERUI_HPP