#ifndef CARSIM_GAMEMANAGER_HPP
#define CARSIM_GAMEMANAGER_HPP



#include "threepp/renderers/GLRenderer.hpp"
#include "threepp/canvas/Canvas.hpp"

#include "core/ResourceManager.hpp"
#include "logic/GameLogicSystem.hpp"
#include "scenes/MapMakerScene.hpp"
#include "UIManager.hpp"
#include "scenes/GameScene.hpp"
#include "scenes/MenuScene.hpp"
#include "core/InputManager.hpp"
#include "controllers/Commands.hpp"

enum class AppState {
    MENU,
    PLAYING,
    PAUSED,
    MAKER
};


class GameManager {
public:
    explicit GameManager(threepp::Canvas& canvas);

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

    [[nodiscard]] int getAiCount() const;
    void setAiCount(int aiCount);

    void update(float dt);
    void handleUI(float dt);
    void render(threepp::GLRenderer& renderer) const;


private:
    AppState state_ = AppState::MENU;
    float globScale = 8.f;
    int aiCount_ = 0;

    // std::unique_ptr<IScene> currentScene_;
    // std::unique_ptr<ILogic> currentLogic_;
    // std::unique_ptr<ILogicSnapshot> currentLogicSnapshot_;

    std::unique_ptr<GameLogicSystem> gameSystem_;
    GameSnapShot gameSnapShot_;
    std::unique_ptr<GameScene> gameScene_;

    std::unique_ptr<MapMakerSystem> mapMakerSystem_;
    MapMakerSnapshot mapMakerSnapShot_;
    std::unique_ptr<MapMakerScene> mapMakerScene_;
    std::unique_ptr<MenuScene> menuScene_;

    UIManager uiManager_;
    InputManager inputManager_;
    EventBus bus_;
    std::unique_ptr<ResourceManager> resources;
};

#endif // CARSIM_GAMEMANAGER_HPP
