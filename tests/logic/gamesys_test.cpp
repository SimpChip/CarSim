#include "catch2/catch_test_macros.hpp"
#include "logic/GameLogicSystem.hpp"
#include "types/mapConfig.hpp"

std::shared_ptr<MapConfig> makeTestMapConfig() {
    auto cfg = std::make_shared<MapConfig>();
    cfg->name = "map";
    cfg->type = "test";

    const std::vector points = {
        threepp::Vector3{0, 0, 0},
        threepp::Vector3{10, 10, 10},
        threepp::Vector3{20, 20, 20},
        threepp::Vector3{40, 40, 40},
        threepp::Vector3{50, 50, 50}
    };
    cfg->roadPoints = points;
    cfg->roadWidth = 10;
    cfg->looped = false;
    cfg->checkpoints = 4;
    return cfg;
}

TEST_CASE("gamesys_minimal_world", "[GameLogicSystem]") {
    EventBus bus;
    auto cfg = makeTestMapConfig();

    GameLogicSystem game(bus, 1, 0, cfg);
    auto snap = game.snapshot();
    REQUIRE(snap.cars.size() == 2);
    REQUIRE(!snap.map.roadSamples.empty());
    REQUIRE(snap.lap.checkpoints.size() == 4);
}