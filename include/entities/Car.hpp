#ifndef CARSIM_CARLOGIC_HPP
#define CARSIM_CARLOGIC_HPP

#include "collision/ICollidable.hpp"
#include "types/gameSnapshots.hpp"

class Car : public ICollidable {
public:
    Car(const threepp::Vector3& startPos, float rotation);
    void update(float dt);
    void applyVelocity(const threepp::Vector3& vel);

    [[nodiscard]] threepp::Vector3 getVelocity() const;
    [[nodiscard]] threepp::Vector3 forwardVector() const;
    [[nodiscard]] CarSnap snapshot() const;

    [[nodiscard]] OBB2D getBoundingBox() const override;
    void onCollision(const std::shared_ptr<ICollidable>& other, float dt) override;

    void setDragMultiplier(float m);
    void setGripMultiplier(float m);
    void setRollingResistanceMultiplier(float m);
    void setMaxSpeedMultiplier(float m);

    void setSteeringLeftActive(bool state);
    void setSteeringRightActive(bool state);
    void setAccelerationInput(float amount);

private:
    threepp::Vector3 size_ = {20, 40, 15};
    threepp::Vector3 velocity_{0,0,0};

    float accelerationSpeed_, brakeSpeed_, baseMaxSpeed_;
    float steeringEaseRate_ = 6.f;

    float rollingResistance_, drag_, basegrip_, driftSlide_;

    float dragMultiplier_ = 1.0f;
    float gripMultiplier_ = 1.0f;
    float rollingResistanceMultiplier_ = 1.0f;
    float maxSpeedMultiplier_ = 1.0f;
    float boostFactor_ = 1;

    float accelerationInput_ = 0;
    float targetSteeringInput_ = 0.0f;
    float currentSteeringInput_ = 0.0f;
    bool isSteeringLeftActive_ = false;
    bool isSteeringRightActive_ = false;

    void updateTargetSteering();
    void updateRotation(float dt);
    void updateVelocity(float dt);
    void applySideFriction(float dt);
    void clampSpeed();
    void resolveCollisionImpulse(const std::shared_ptr<Car>& other, float dt);
};


#endif //CARSIM_CARLOGIC_HPP