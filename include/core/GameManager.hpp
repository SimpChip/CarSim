#ifndef CARSIM_GAMEMANAGER_HPP
#define CARSIM_GAMEMANAGER_HPP

#include "helpers/ResourceManager.hpp"
#include "core/UIManager.hpp"
#include "core/InputManager.hpp"
#include "controllers/Commands.hpp"
#include "logic/GameLogicSystem.hpp"
#include "scenes/MenuScene.hpp"
#include "scenes/GameScene.hpp"
#include "scenes/MapMakerScene.hpp"

#include "types/gameSnapshots.hpp"
#include "types/mapMakerSnapshots.hpp"

enum class AppState {
    MENU,
    PLAYING,
    PAUSED,
    MAKER
};


class GameManager {
public:
    explicit GameManager(threepp::Canvas* canvas);

    GameCommands commands() {
        return {
            [this](const std::string& map, const int aiCount, const std::string& model) {
                startRace(map, aiCount, model);
            },
            [this]() { startMapMaker(); },
            [this]() { pause(); },
            [this]() { resume(); },
            [this]() { startMenu(); }
        };
    }

    void startMenu();
    void startRace(const std::string& map,int aiCount, const std::string& model);
    void startMapMaker();
    void pause();
    void resume();


    void subscribe();

    void setAiCount(int aiCount);

    void update(float dt);
    void handleUI(float dt);
    void render(threepp::GLRenderer& renderer) const;


private:
    AppState state_ = AppState::MENU;
    float globScale = 8.f;
    int aiCount_ = 0;

    std::unique_ptr<GameLogicSystem> gameSystem_;
    GameSnap gameSnap_;
    std::unique_ptr<GameScene> gameScene_;


    //TODO MapMaker not implmented fully yet
    std::unique_ptr<MapMakerSystem> mapMakerSystem_;
    MapMakerSnap mapMakerSnap_;
    std::unique_ptr<MapMakerScene> mapMakerScene_;

    std::unique_ptr<MenuScene> menuScene_;

    UIManager uiManager_;
    InputManager inputManager_;
    EventBus bus_;
    std::unique_ptr<ResourceManager> resources;
};

#endif // CARSIM_GAMEMANAGER_HPP
