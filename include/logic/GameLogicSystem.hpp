#ifndef CARSIM_GAMELOGIC_HPP
#define CARSIM_GAMELOGIC_HPP

#include <memory>
#include "collision/CollisonManager.hpp"
#include "MapSystem.hpp"
#include "LapSystem.hpp"
#include "PickupSystem.hpp"
#include "CarSystem.hpp"
#include "TrackGenerator.hpp"
#include "types/gameSnapshots.hpp"

class GameLogicSystem {
public:
    GameLogicSystem(EventBus& bus,int aiCount, int startIdx, const std::shared_ptr<MapConfig>& mapConfig);
    void update(float dt);
    GameSnap snapshot();
private:
    float globScale_ = 8;
    int startIdx_ = 0;
    CarSystem carSystem_;
    MapSystem mapSystem_;
    PickupSystem pickupSystem_;
    LapSystem lapSystem_;
    CollisionManager collisionManager_;
    TrackGenerator trackGenerator_;

    void generateWorld(int aiCount, int startIdx, int numCp);
    void registerAllSystems();
};


#endif //CARSIM_GAMELOGIC_HPP