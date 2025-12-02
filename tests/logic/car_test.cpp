#include "entities/Car.hpp"
#include "../../src/entities/Car.cpp"
#include <catch2/catch_test_macros.hpp>
#include "catch2/catch_approx.hpp"

//size standard is right now 20, 40, 15
struct CarTest {
    Car car;
    CarTest() : car(threepp::Vector3{0,0,0}, 0.0f) {}
};

struct CarZeroGravity {
    Car car;
    CarZeroGravity() : car(threepp::Vector3{0,0,0}, 0.0f) {
        car.setDragMultiplier(0.0f);
        car.setGripMultiplier(0.0f);
        car.setRollingResistanceMultiplier(0.0f);
        car.setMaxSpeedMultiplier(0.0f);
    }
};

TEST_CASE_METHOD(CarTest, "accell_and_brake_cycle", "[car]") {
    car.setAccelerationInput(1.0f);
    for(int i=0; i<10; i++) car.update(1.f);
    const auto peakSpeed = car.getVelocity().length();
    REQUIRE(peakSpeed > 0.0f);

    car.setAccelerationInput(0.0f);

    double prevSpeed = peakSpeed;
    bool slowedDown = false;

    for(int i=0; i<20; i++) {
        car.update(1.f);
        const auto currentSpeed = car.getVelocity().length();
        REQUIRE(currentSpeed <= prevSpeed + 1e-6);
        if(currentSpeed < peakSpeed * 0.5) slowedDown = true;

        prevSpeed = currentSpeed;
    }
    REQUIRE(slowedDown);
}

TEST_CASE_METHOD(CarTest, "steer_left", "[car]") {
    car.setSteeringLeftActive(true);
    const auto noMoveRotation = car.getRotation();
    for(int i=0;i<10;i++) car.update(0.1f);
    const auto noMoveNewRotation = car.getRotation();
    REQUIRE(noMoveRotation == noMoveNewRotation);

    car.setAccelerationInput(1.0f);
    const auto startPosition = car.getPosition();
    for(int i=0;i<10;i++) {
        car.update(0.1f);
    }
    const auto withMoveNewRotation = car.getRotation();
    REQUIRE(withMoveNewRotation < noMoveRotation);
    const auto newPosition = car.getPosition();
    REQUIRE(newPosition.x < startPosition.x);
}

TEST_CASE_METHOD(CarTest, "steer_right", "[car]") {
    car.setSteeringRightActive(true);
    const auto noMoveRotation = car.getRotation();
    for(int i=0;i<10;i++) car.update(0.1f);
    const auto noMoveNewRotation = car.getRotation();
    REQUIRE(noMoveRotation == noMoveNewRotation);

    car.setAccelerationInput(1.0f);
    const auto startPosition = car.getPosition();
    for(int i=0;i<10;i++) {
        car.update(0.1f);
    }
    const auto withMoveNewRotation = car.getRotation();
    REQUIRE(withMoveNewRotation > noMoveRotation);
    const auto newPosition = car.getPosition();
    REQUIRE(newPosition.x > startPosition.x);
}

TEST_CASE_METHOD(CarZeroGravity, "zero_gravity", "[car]") {
    car.applyVelocity({50,50,0});
    auto prevSpeed = car.getVelocity().length();
    for(int i=0;i<10;i++) {
        REQUIRE(car.getVelocity().length() == prevSpeed);
        prevSpeed = car.getVelocity().length();
        car.update(0.1f);
    }
}


TEST_CASE_METHOD(CarZeroGravity, "drag_effect", "[car]") {
    car.applyVelocity({10,0,0});
    car.setDragMultiplier(3.0f);
    car.update(0.1f);
    REQUIRE(car.getVelocity().length() < 10.0f);
}

TEST_CASE_METHOD(CarZeroGravity, "rolling_resistance_effect", "[car]") {
    car.applyVelocity({10,0,0});
    car.setRollingResistanceMultiplier(3.0f);
    car.update(0.1f);
    REQUIRE(car.getVelocity().length() < 10.0f);
}

TEST_CASE_METHOD(CarZeroGravity, "max_speed_limit", "[car]") {
    car.setMaxSpeedMultiplier(0.0001f);
    car.setAccelerationInput(1.0f);
    for(int i=0;i<200;i++) car.update(0.016f);
    REQUIRE(car.getVelocity().length() < 5.0f);
}

TEST_CASE_METHOD(CarTest, "snapshot_consistency", "[car]") {
    car.setAccelerationInput(1.0f);
    for(int i=0;i<30;i++) car.update(0.016f);
    auto snap1 = car.snapshot();

    car.setSteeringLeftActive(true);
    for(int i=0;i<30;i++) car.update(0.016f);
    auto snap2 = car.snapshot();

    REQUIRE(snap1.position != snap2.position);
    REQUIRE(snap2.speed > snap1.speed);
}

TEST_CASE_METHOD(CarZeroGravity, "head_on_collision_response", "[car]") {
    const auto carA = std::make_shared<Car>(threepp::Vector3{0,0,0}, 0.0f);
    const auto carB = std::make_shared<Car>(threepp::Vector3{2,0,0}, 0.0f);

    carA->applyVelocity({5,0,0});
    carB->applyVelocity({-5,0,0});

    carA->update(0.1f);
    carB->update(0.1f);

    carA->onCollision(carB, 0.1f);
    carB->onCollision(carA, 0.1f);

    REQUIRE(carA->getVelocity().x < 5.0f);
    REQUIRE(carB->getVelocity().x < 5.0f);
}