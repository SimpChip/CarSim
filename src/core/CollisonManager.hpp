//
// Created by Mella on 26.11.2025.
//

#ifndef CARSIM_COLLISONMANAGER_HPP
#define CARSIM_COLLISONMANAGER_HPP

#include "data/SpatialGrid.hpp"
#include "data/OBB.hpp"
#include "types/data.hpp"
#include "set"


class ICollidable {
public:
    virtual ~ICollidable() = default;
    [[nodiscard]] virtual CollisionType getCollisonType() const = 0;
    [[nodiscard]] virtual OBB getBoundingBox() const = 0;
    [[nodiscard]] virtual threepp::Vector3 getPosition() const = 0;
    virtual void onCollision(ICollidable* other) = 0;
};

class CollisionManager {
public:
    void clear() { carGrid_.grid.clear(); }
    void registerCollidable(ICollidable* obj) {
        carGrid_.insert(obj->getPosition(), obj);
    }

    std::vector<std::pair<ICollidable*, ICollidable*>> detectCollisions() {
        std::vector<std::pair<ICollidable*, ICollidable*>> collisions;
        std::set<std::pair<const ICollidable*, const ICollidable*>> checked;

        for (auto& [cell, collidables] : carGrid_.grid) {
            for (size_t i = 0; i < collidables.size(); ++i) {
                for (size_t j = i + 1; j < collidables.size(); ++j) {
                    checkAndAdd(collidables[i], collidables[j], collisions, checked);
                }
            }
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (dx == 0 && dy == 0) continue;
                    auto neighbor = std::make_pair(cell.first + dx, cell.second + dy);
                    auto it = carGrid_.grid.find(neighbor);
                    if (it == carGrid_.grid.end()) continue;
                    for (auto* A : collidables) {
                        for (auto* B : it->second) {
                            checkAndAdd(A, B, collisions, checked);
                        }
                    }
                }
            }
        }
        return collisions;
    }

    static void resolveCollisions(const std::vector<std::pair<ICollidable*, ICollidable*>>& collisions) {
        for (auto& [A, B] : collisions) {
            A->onCollision(B);
            B->onCollision(A);
        }
    }

private:
    SpatialGrid<ICollidable*> carGrid_{250.f};

    static void checkAndAdd(ICollidable* A, ICollidable* B,
                            std::vector<std::pair<ICollidable*, ICollidable*>>& collisions,
                            std::set<std::pair<const ICollidable*, const ICollidable*>>& checked) {
        const auto key = std::minmax(A, B); // ensures consistent ordering
        if (checked.contains(key)) return;
        checked.insert(key);

        if (A->getBoundingBox().intersects(B->getBoundingBox())) {
            collisions.emplace_back(A, B);
        }
    }
};

#endif //CARSIM_COLLISONMANAGER_HPP