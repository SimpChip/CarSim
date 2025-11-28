//
// Created by Mella on 27.11.2025.
//

#ifndef CARSIM_ICOLLIDABLE_HPP
#define CARSIM_ICOLLIDABLE_HPP

#include <atomic>
#include "threepp/math/Vector2.hpp"
#include "threepp/math/Vector3.hpp"
#include "types/data.hpp"
#include "OBB2D.hpp"
#include "memory"




struct IdGenerator {
    std::atomic<int> defualtCounter{0};
    std::atomic<int> carCounter{0};
    std::atomic<int> pickupCounter{0};
    std::atomic<int> checkpointCounter{0};
    std::atomic<int> obstacleCounter{0};


    int next(CollisionType ns) {
        switch(ns) {
            case CollisionType::Car:            return 200000 + carCounter++;
            case CollisionType::Checkpoint:      return 300000 + checkpointCounter++;
            case CollisionType::SlowBoost:      return 400000 + pickupCounter++;
            case CollisionType::SpeedBoost:    return 600000  + pickupCounter++;
            case CollisionType::Obstacle:       return 800000 + obstacleCounter++;
            default: return 0 + defualtCounter++;
        }
    }
};

inline IdGenerator globalIdGen;

class ICollidable {
public:
    explicit ICollidable(const threepp::Vector3 position, CollisionType type) : position_(position), prevPosition_(position) {
        id_ = globalIdGen.next(type);
        type_ = type;
    }

    virtual ~ICollidable() = default;
    [[nodiscard]] int getId() const { return id_; }

    [[nodiscard]] threepp::Vector2 getPosition2D() const {return {position_.x, position_.y};}
    [[nodiscard]] threepp::Vector2 getPrevPosition2D() const {return {position_.x, position_.y};}

    [[nodiscard]] threepp::Vector3 getPosition() const {return position_;}
    [[nodiscard]] threepp::Vector3 getPrevPosition() const {return prevPosition_;}
    [[nodiscard]] CollisionType getCollisionType() const {return type_;};
    [[nodiscard]] virtual OBB2D getBoundingBox() const = 0;

    virtual void onCollision(const std::shared_ptr<ICollidable>& other) = 0;

protected:
    int id_;
    CollisionType type_;
    threepp::Vector3 position_;
    threepp::Vector3 prevPosition_;

private:
    static int generateUniqueId() {
        static std::atomic counter{1};
        return counter++;
    }
};


#endif //CARSIM_ICOLLIDABLE_HPP