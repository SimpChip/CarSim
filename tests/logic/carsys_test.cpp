
#include "catch2/catch_test_macros.hpp"
#include "logic/CarSystem.hpp"
#include "controllers/PlayerController.hpp"
#include "core/EventBus.hpp"

struct CarSysTest {
    EventBus bus;
    CarSystem sys;
    CarSysTest() : sys(bus) {
    }
};

TEST_CASE_METHOD(CarSysTest, "add_player_car", "[carsystem]") {
    sys.addPlayerCar({0,0,0}, 0.0f);
    auto& player = sys.getPlayerCar();
    REQUIRE(player.controller != nullptr);
    REQUIRE(dynamic_cast<PlayerController*>(player.controller.get()) != nullptr);
    REQUIRE(sys.getPlayerCarID() == player.logic->getId());
    REQUIRE(dynamic_cast<Car*>(player.logic.get()) != nullptr);
    REQUIRE(sys.getCars().size() == 1);
}

TEST_CASE_METHOD(CarSysTest, "CarSystem maintains player car index", "[carsystem]") {
    sys.addPlayerCar({0,0,0}, 0.0f);
    constexpr std::vector<RoadSample> samples;
    sys.addAICar({20,0,0}, 0.0f, samples, 0, 1.0f);
    const auto& player = sys.getPlayerCar();
    REQUIRE(player.logic->getId() == sys.getPlayerCarID());
}