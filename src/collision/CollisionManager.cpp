#include <ranges>

#include "collision/CollisonManager.hpp"
#include "iostream"
#include "types/events.hpp"
#include "collision/OBB2D.hpp"


CollisionManager::CollisionManager(EventBus& bus) : bus_(bus) {}

void CollisionManager::registerCar(const std::shared_ptr<ICollidable>& obj) {
    if (!obj) return;
    if (objects_.contains(obj->getId())) {
        std::cerr << "Duplicate ID detected: " << obj->getId() << "\n";
    }
    objects_[obj->getId()] = obj;
    updateObjectCells(carGrid_, obj);
}
void CollisionManager::registerDynamic(const std::shared_ptr<ICollidable>& obj) {
    if (!obj) return;
    if (objects_.contains(obj->getId())) {
        std::cerr << "Duplicate ID detected: " << obj->getId() << "\n";
    }
    objects_[obj->getId()] = obj;
    updateObjectCells(dynamicGrid_, obj);
}

void CollisionManager::registerStatic(const std::shared_ptr<ICollidable>& obj) {
    if (!obj) return;
    if (objects_.contains(obj->getId())) {
        std::cerr << "Duplicate ID detected: " << obj->getId() << "\n";
    }
    objects_[obj->getId()] = obj;
    updateObjectCells(staticGrid_, obj);
}

void CollisionManager::updateDynamicPosition(const std::shared_ptr<ICollidable>& obj) {
    if (!obj) return;
    updateObjectCells(dynamicGrid_, obj);
}

void CollisionManager::updateCarPosition(const std::shared_ptr<ICollidable>& obj) {
    if (!obj) return;
    updateObjectCells(carGrid_, obj);
}

std::shared_ptr<ICollidable> CollisionManager::getObjectById(const int id) const {
    const auto it = objects_.find(id);
    if (it == objects_.end()) return nullptr;
    return it->second;
}


void CollisionManager::detectCollisions(const float dt) {
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

void CollisionManager::checkGrid(SpatialGrid2D<int>& grid, float dt) {
    for (auto& ids : grid.grid | std::views::values) {
        const size_t n = ids.size();
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                checkAndHandle(ids[i], ids[j], dt);
            }
        }
    }
}

void CollisionManager::checkGridPair(SpatialGrid2D<int>& gridA, SpatialGrid2D<int>& gridB, float dt) {
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

void CollisionManager::checkAndHandle(const int idA, const int idB, float dt) {
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

std::vector<CollisionManager::CellKey> CollisionManager::cellsForOBB(const SpatialGrid2D<int>& grid, const OBB2D& obb) {
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


void CollisionManager::updateObjectCells(SpatialGrid2D<int>& grid, const std::shared_ptr<ICollidable>& obj) {
    if (!obj) return;
    const int id = obj->getId();
    const Pose2D curPose{ obj->getPosition2D(), obj->getRotation() };
    const auto itPose = lastPose_.find(id);
    if (itPose != lastPose_.end()) {
        const float dpos = (curPose.pos - itPose->second.pos).length();
        const float drot = std::abs(curPose.rot - itPose->second.rot);
        if (dpos <= posThreshold_ && drot <= rotThreshold_) {
            return;
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
    oldSet.swap(newCells);
    lastPose_[id] = curPose;
}

bool CollisionManager::sweptIntersects(const ICollidable& car, const ICollidable& other) {
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