#ifndef CARSIM_COLLISONMANAGER_HPP
#define CARSIM_COLLISONMANAGER_HPP

#include <iostream>
#include <ranges>
#include <unordered_set>
#include "core/EventBus.hpp"
#include "ICollidable.hpp"
#include "SpatialGrid2D.hpp"
#include "OBB2D.hpp"
#include "types/data.hpp"
#include "types/events.hpp"

struct CollisionState {
    bool active{false};
    int exitCountdown{0};
};

struct Pose2D {
    threepp::Vector2 pos;
    float rot{0.f};
};

class CollisionManager {
public:
    explicit CollisionManager(EventBus& bus) : bus_(bus) {};
    void registerCar(const std::shared_ptr<ICollidable>& obj) {
        if (!obj) return;
        if (objects_.contains(obj->getId())) {
            std::cerr << "Duplicate ID detected: " << obj->getId() << "\n";
        }
        objects_[obj->getId()] = obj;
        updateObjectCells(carGrid_, obj);
    }

    void registerDynamic(const std::shared_ptr<ICollidable>& obj) {
        if (!obj) return;
        if (objects_.contains(obj->getId())) {
            std::cerr << "Duplicate ID detected: " << obj->getId() << "\n";
        }
        objects_[obj->getId()] = obj;
        updateObjectCells(dynamicGrid_, obj);
    }

    void registerStatic(const std::shared_ptr<ICollidable>& obj) {
        if (!obj) return;
        if (objects_.contains(obj->getId())) {
            std::cerr << "Duplicate ID detected: " << obj->getId() << "\n";
        }
        objects_[obj->getId()] = obj;
        updateObjectCells(staticGrid_, obj);
    }

    void updateDynamicPosition(const std::shared_ptr<ICollidable>& obj) {
        if (!obj) return;
        updateObjectCells(dynamicGrid_, obj);
    }
    void updateCarPosition(const std::shared_ptr<ICollidable>& obj) {
        if (!obj) return;
        updateObjectCells(carGrid_, obj);
    }

    std::shared_ptr<ICollidable> getObjectById(const int id) const {
        const auto it = objects_.find(id);
        if (it == objects_.end()) return nullptr;
        return it->second;
    }

    void detectCollisions(float dt) {
        checkGrid(carGrid_, dt);
        checkGridPair(carGrid_, dynamicGrid_, dt);
        checkGridPair(carGrid_, staticGrid_, dt);
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
                if (exitCountdown >= exitFrames_) {
                    bus_.publish(CollisionExitEvent{
                           A->getId(), B->getId(), A->getCollisionType(), B->getCollisionType()
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
    EventBus& bus_;
    int exitFrames_{10};
    SpatialGrid2D<int> carGrid_{250.f};
    SpatialGrid2D<int> dynamicGrid_{250.f};
    SpatialGrid2D<int> staticGrid_{250.f};

    std::unordered_map<std::pair<int,int>, CollisionState, PairHash> activeCollisions_;
    std::unordered_map<int, std::shared_ptr<ICollidable>> objects_;

    using CellKey = SpatialGrid2D<int>::Cell;
    using CellKeyHash = SpatialGrid2D<int>::CellHash;
    std::unordered_map<int, std::unordered_set<CellKey, CellKeyHash>> occupiedCells_;
    std::unordered_map<int, Pose2D> lastPose_;
    float posThreshold_{0.5f};
    float rotThreshold_{0.01f};

    void checkGrid(SpatialGrid2D<int>& grid, float dt) {
        for (auto& ids : grid.grid | std::views::values) {
            const size_t n = ids.size();
            for (size_t i = 0; i < n; ++i) {
                for (size_t j = i + 1; j < n; ++j) {
                    checkAndHandle(ids[i], ids[j], dt);
                }
            }
        }
    }

    void checkGridPair(SpatialGrid2D<int>& gridA, SpatialGrid2D<int>& gridB, float dt) {
        for (auto& [cell, idsA] : gridA.grid) {
            auto it = gridB.grid.find(cell);
            if (it == gridB.grid.end()) continue;
            for (const int idA : idsA) {
                for (const int idB : it->second) {
                    checkAndHandle(idA, idB, dt);
                }
            }
        }
    }

    void checkAndHandle(const int idA, const int idB, float dt) {
        if (idA == idB) return;
        const auto itA = objects_.find(idA);
        const auto itB = objects_.find(idB);
        if (itA == objects_.end() || itB == objects_.end()) return;
        const auto A = itA->second;
        const auto B = itB->second;
        if (!A || !B) return;
        if (!A->isActive() || !B->isActive()) return;

        const auto key = std::minmax(A->getId(), B->getId());

        const OBB2D boxA = A->getBoundingBox();
        const OBB2D boxB = B->getBoundingBox();

        bool intersects = boxA.intersects(boxB);

        if (!intersects) {
            intersects = sweptIntersects(*A, *B) || sweptIntersects(*B, *A);
        }
        auto& [active, exitCountdown] = activeCollisions_[key];
        if (intersects) {

            if (A->getCollisionType() == CollisionType::Car &&
                B->getCollisionType() == CollisionType::Car) {
                bus_.publish(CollisionEnterEvent{
                    A->getId(), B->getId(), A->getCollisionType(), B->getCollisionType()
                });
                A->onCollision(B, dt);
                B->onCollision(A, dt);
                return;
            }
            if (!active) {
                bus_.publish(CollisionEnterEvent{
                    A->getId(), B->getId(), A->getCollisionType(), B->getCollisionType()
                });
                active = true;
                exitCountdown = 0;
            }
            A->onCollision(B, dt);
            B->onCollision(A, dt);
        }
    }

    static std::vector<CellKey> cellsForOBB(const SpatialGrid2D<int>& grid, const OBB2D& obb) {
        const auto corners = obb.getCorners();
        float minX = corners[0].x, maxX = corners[0].x;
        float minY = corners[0].y, maxY = corners[0].y;
        for (size_t i = 1; i < corners.size(); ++i) {
            minX = std::min(minX, corners[i].x);
            maxX = std::max(maxX, corners[i].x);
            minY = std::min(minY, corners[i].y);
            maxY = std::max(maxY, corners[i].y);
        }

        const CellKey minCell = grid.cellFor({minX, minY});
        const CellKey maxCell = grid.cellFor({maxX, maxY});

        std::vector<CellKey> out;
        out.reserve((maxCell.x - minCell.x + 1) * (maxCell.y - minCell.y + 1));
        for (int cy = minCell.y; cy <= maxCell.y; ++cy) {
            for (int cx = minCell.x; cx <= maxCell.x; ++cx) {
                out.push_back({cx, cy});
            }
        }
        return out;
    }

    void updateObjectCells(SpatialGrid2D<int>& grid, const std::shared_ptr<ICollidable>& obj) {
        if (!obj) return;
        const int id = obj->getId();
        Pose2D curPose{ obj->getPosition2D(), obj->getRotation() };
        auto itPose = lastPose_.find(id);
        if (itPose != lastPose_.end()) {
            const float dpos = (curPose.pos - itPose->second.pos).length();
            const float drot = std::abs(curPose.rot - itPose->second.rot);
            if (dpos <= posThreshold_ && drot <= rotThreshold_) {
                return; // no significant change
            }
        }

        const OBB2D obb = obj->getBoundingBox();
        const auto newCellsVec = cellsForOBB(grid, obb);
        std::unordered_set<CellKey, CellKeyHash> newCells(newCellsVec.begin(), newCellsVec.end());

        auto &oldSet = occupiedCells_[id];

        for (const auto& oldKey : oldSet) {
            if (!newCells.contains(oldKey)) {
                grid.removeCell(oldKey, id);
            }
        }

        for (const auto& newKey : newCells) {
            if (!oldSet.contains(newKey)) {
                grid.insertCell(newKey, id);
            }
        }

        // store new set and pose
        oldSet.swap(newCells);
        lastPose_[id] = curPose;
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