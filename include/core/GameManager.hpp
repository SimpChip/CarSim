#ifndef CARSIM_GAMEMANAGER_HPP
#define CARSIM_GAMEMANAGER_HPP


#include "threepp/renderers/GLRenderer.hpp"
#include "threepp/canvas/Canvas.hpp"

#include "core/ResourceManager.hpp"
#include "scenes/GameScene.hpp"
#include "scenes/MenuScene.hpp"
#include "logic/GameLogic.hpp"
#include "scenes/MapCreatorScene.hpp"

#include "controllers/PlayerController.hpp"
#include "core/UiManager.hpp"
#include "types/data.hpp"

enum class AppState {
    MENU,
    PLAYING,
    STARTING,
    PAUSED,
    MAKER
};

class GameManager {
public:
    GameManager();

    void attachListeners(threepp::Canvas& canvas);

    void startMenu();
    void startRace(const std::string& mapName, const std::string& modelName);
    void startMapMaker();

    void pauseGame();
    void resumeGame();

    void setStartTimer(float startTimer);

    [[nodiscard]] int getAiCount() const;
    void setAiCount(int aiCount);

    void onKeyPressed(const threepp::KeyEvent& evt);
    void onKeyReleased(const threepp::KeyEvent& evt) const;

    void update(float dt);
    void handleUI(UIManager& ui, float dt);
    void render(threepp::GLRenderer& renderer) const;

    [[nodiscard]] PlayerController* playerController() const;

private:
    threepp::KeyAdapter keyPressed_;
    threepp::KeyAdapter keyReleased_;
    GameState gameSnapshot_;
    AppState state_ = AppState::MENU;
    float globScale = 8.f;
    float startTimer_ = 0;
    int aiCount_ = 0;

    std::unique_ptr<ResourceManager> resources;
    std::unique_ptr<GameLogic> gameLogic;
    std::unique_ptr<GameScene> gameScene_;
    std::unique_ptr<MenuScene> menuScene_;
    std::unique_ptr<MapCreatorLogic> mapCreatorLogic;
};

#endif // CARSIM_GAMEMANAGER_HPP
