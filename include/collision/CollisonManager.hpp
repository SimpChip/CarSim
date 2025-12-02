#ifndef CARSIM_COLLISONMANAGER_HPP
#define CARSIM_COLLISONMANAGER_HPP

#include <unordered_set>
#include "core/EventBus.hpp"
#include "ICollidable.hpp"
#include "SpatialGrid2D.hpp"
#include "OBB2D.hpp"
#include "types/data.hpp"

struct CollisionActiveState {
    bool active{false};
    int exitCountdown{0};
};

struct Pose2D {
    threepp::Vector2 pos;
    float rot{0.f};
};

class CollisionManager {
public:
    explicit CollisionManager(EventBus& bus);
    void registerCar(const std::shared_ptr<ICollidable>& obj);

    void registerDynamic(const std::shared_ptr<ICollidable>& obj);

    void registerStatic(const std::shared_ptr<ICollidable>& obj);

    void updateDynamicPosition(const std::shared_ptr<ICollidable>& obj);

    void updateCarPosition(const std::shared_ptr<ICollidable>& obj);

    std::shared_ptr<ICollidable> getObjectById(int id) const;

    void detectCollisions(float dt);

private:
    EventBus& bus_;
    int exitFrames_{10};
    SpatialGrid2D<int> carGrid_{250.f};
    SpatialGrid2D<int> dynamicGrid_{250.f};
    SpatialGrid2D<int> staticGrid_{250.f};

    std::unordered_map<std::pair<int,int>, CollisionActiveState, PairHash> activeCollisions_;
    std::unordered_map<int, std::shared_ptr<ICollidable>> objects_;

    using CellKey = SpatialGrid2D<int>::Cell;
    using CellKeyHash = SpatialGrid2D<int>::CellHash;
    std::unordered_map<int, std::unordered_set<CellKey, CellKeyHash>> occupiedCells_;
    std::unordered_map<int, Pose2D> lastPose_;
    float posThreshold_{0.5f};
    float rotThreshold_{0.01f};

    void checkGrid(SpatialGrid2D<int>& grid, float dt);

    void checkGridPair(SpatialGrid2D<int>& gridA, SpatialGrid2D<int>& gridB, float dt);

    void checkAndHandle(int idA, int idB, float dt);

    static std::vector<CellKey> cellsForOBB(const SpatialGrid2D<int>& grid, const OBB2D& obb);

    void updateObjectCells(SpatialGrid2D<int>& grid, const std::shared_ptr<ICollidable>& obj);

    static bool sweptIntersects(const ICollidable& car, const ICollidable& other);
};

#endif //CARSIM_COLLISONMANAGER_HPP