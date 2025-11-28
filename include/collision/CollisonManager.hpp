//
// Created by Mella on 26.11.2025.
//

#ifndef CARSIM_COLLISONMANAGER_HPP
#define CARSIM_COLLISONMANAGER_HPP


#include <ranges>

#include "EventBus.hpp"
#include "ICollidable.hpp"
#include "SpatialGrid2D.hpp"
#include "OBB2D.hpp"
#include "types/data.hpp"
#include "map"



struct CollisionState {
    bool active{false};
    int exitCountdown{0};
};

class CollisionManager {
public:
    void clearDynamic() { carGrid_.grid.clear(); dynamicGrid_.grid.clear(); }

    void registerCar(const std::shared_ptr<ICollidable>& obj) {
        if (objects_.contains(obj->getId())) {
            std::cerr << "Duplicate ID detected: " << obj->getId() << "\n";
        }
        carGrid_.insert(obj->getPosition2D(), obj->getId());
        objects_[obj->getId()] = obj;
    }
    void registerDynamic(const std::shared_ptr<ICollidable>& obj) {
        if (objects_.contains(obj->getId())) {
            std::cerr << "Duplicate ID detected: " << obj->getId() << "\n";
        }
        dynamicGrid_.insert(obj->getPosition2D(), obj->getId());
        objects_[obj->getId()] = obj; }
    void registerStatic(const std::shared_ptr<ICollidable>& obj) {
        if (objects_.contains(obj->getId())) {
            std::cerr << "Duplicate ID detected: " << obj->getId() << "\n";
        }
        staticGrid_.insert(obj->getPosition2D(), obj->getId());
        objects_[obj->getId()] = obj;
    }

    void updateDynamicPosition(const std::shared_ptr<ICollidable>& obj) {
        dynamicGrid_.insert(obj->getPosition2D(), obj->getId());
    }
    void updateCarPosition(const std::shared_ptr<ICollidable>& obj) {
        carGrid_.insert(obj->getPosition2D(), obj->getId());
    }

    std::shared_ptr<ICollidable> getObjectById(const int id) const {
        const auto it = objects_.find(id);
        if (it == objects_.end()) return nullptr;
        return it->second;
    }

    void detectCollisions() {
        checkGrid(carGrid_);
        checkGridPair(carGrid_, dynamicGrid_);
        checkGridPair(carGrid_, staticGrid_);

        for (auto it = activeCollisions_.begin(); it != activeCollisions_.end();) {
            const auto& [idA, idB] = it->first;
            const auto A = objects_[idA];
            const auto B = objects_[idB];

            if (!A || !B) {
                it = activeCollisions_.erase(it);
                continue;
            }

            const bool intersects = A->getBoundingBox().intersects(B->getBoundingBox());
            auto& [active, exitCountdown] = it->second;

            if (intersects) { exitCountdown = 0; } else if (active) {
                exitCountdown++;
                if (exitCountdown >= exitDebounceFrames_) {
                    EventBus::publish(CollisionExitEvent{
                        A->getId(), B->getId(),
                        A->getCollisionType(), B->getCollisionType()
                    });
                    active = false;
                    exitCountdown = 0;
                    it = activeCollisions_.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

private:
    int exitDebounceFrames_{4};

    SpatialGrid2D<int> carGrid_{250.f};
    SpatialGrid2D<int> dynamicGrid_{250.f};
    SpatialGrid2D<int> staticGrid_{250.f};

    std::unordered_map<std::pair<int,int>, CollisionState, PairHash> activeCollisions_;
    std::unordered_map<int, std::shared_ptr<ICollidable>> objects_;

    void checkGrid(SpatialGrid2D<int>& grid) {
        for (auto& ids : grid.grid | std::views::values) {
            const size_t n = ids.size();
            for (size_t i = 0; i < n; ++i) {
                for (size_t j = i + 1; j < n; ++j) {
                    checkAndHandle(ids[i], ids[j]);
                }
            }
        }
    }

    void checkGridPair(SpatialGrid2D<int>& gridA, SpatialGrid2D<int>& gridB) {
        for (auto& [cell, idsA] : gridA.grid) {
            auto it = gridB.grid.find(cell);
            if (it == gridB.grid.end()) continue;
            for (const int idA : idsA) {
                for (const int idB : it->second) {
                    checkAndHandle(idA, idB);
                }
            }
        }
    }

    void checkAndHandle(const int idA, const int idB) {
        if (idA == idB) return;
        const auto A = objects_[idA];
        const auto B = objects_[idB];
        const auto key = std::minmax(A->getId(), B->getId());
        bool intersects = A->getBoundingBox().intersects(B->getBoundingBox());

        if (!intersects) {
            intersects = sweptIntersects(*A, *B) || sweptIntersects(*B, *A);
        }
        auto&[active, exitCountdown] = activeCollisions_[key];
        if (intersects) {
            if (!active) {
                EventBus::publish(CollisionEnterEvent{
                    A->getId(), B->getId(), A->getCollisionType(), B->getCollisionType()
                });
                active = true;
                exitCountdown = 0;
            }
            A->onCollision(B);
            B->onCollision(A);
        }
    }

    static bool sweptIntersects(const ICollidable& car, const ICollidable& other) {
        const auto oldPos = car.getPrevPosition2D();
        const auto newPos = car.getPosition2D();

        constexpr int steps = 5;
        for (int i = 0; i <= steps; ++i) {
            const float t = i / static_cast<float>(steps);
            threepp::Vector2 sample = oldPos + (newPos - oldPos) * t;
            if (other.getBoundingBox().containsPoint(sample)) {
                return true;
            }
        }
        return false;
    }

    static const char* toCollisonString(CollisionType type) {
        switch (type) {
            case CollisionType::Car: return "Car";
            case CollisionType::Checkpoint: return "Checkpoint";
            case CollisionType::SpeedBoost: return "Pickup";
            case CollisionType::Obstacle: return "Obstacle";
            default: return "Unknown";
        }
    }
};

#endif //CARSIM_COLLISONMANAGER_HPP