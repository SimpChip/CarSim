
#include "core/GameManager.hpp"
#include "logic/GameLogicSystem.hpp"
#include "scenes/MenuScene.hpp"
#include "scenes/GameScene.hpp"
#include "types/SnapShots.hpp"
#include "types/events.hpp"

GameManager::GameManager(threepp::Canvas& canvas )
    : gameSnapShot_(), mapMakerSnapShot_(), uiManager_(canvas.windowPtr()), inputManager_(bus_),
      resources(std::make_unique<ResourceManager>()) {
    canvas.addKeyListener(inputManager_);
    subscribe();
    startMenu();
}

void GameManager::subscribe() {
    bus_.subscribe<PauseEvent>([this](const PauseEvent&) {
        if (state_ == AppState::PLAYING) {
            pause();
        } else if (state_ == AppState::PAUSED) {
            resume();
        }
    });
}


void GameManager::startMenu() {
    menuScene_ = std::make_unique<MenuScene>(*resources);
    state_ = AppState::MENU;
}
void GameManager::startRace(const std::string& map, int aiCount, const std::string& model) {
    bus_ = EventBus{};
    subscribe();
    gameSystem_ = std::make_unique<GameLogicSystem>(bus_, *resources, map, aiCount, 0);
    gameSnapShot_ = gameSystem_->snapshot();
    gameScene_ = std::make_unique<GameScene>(gameSnapShot_, *resources, model);
    state_ = AppState::PLAYING;
}
void GameManager::startMapMaker() {
    mapMakerSystem_ = std::make_unique<MapMakerSystem>();
    mapMakerScene_ = std::make_unique<MapMakerScene>(*resources);
    state_ = AppState::MAKER;
}
void GameManager::pause() {
    state_ = AppState::PAUSED;
}
void GameManager::resume() {
    state_ = AppState::PLAYING;
}

int GameManager::getAiCount() const { return aiCount_; }
void GameManager::setAiCount(const int aiCount) { aiCount_ = aiCount; }

void GameManager::update(const float dt) {
    switch (state_) {
        case AppState::PLAYING: {
            gameSystem_->update(dt);
            gameSnapShot_ = gameSystem_->snapshot();
            gameScene_->update(gameSystem_->snapshot(), dt);
            break;
        }
        case AppState::MAKER: {
            mapMakerSystem_->update(dt);
            mapMakerSnapShot_ = mapMakerSystem_->snapshot();
            mapMakerScene_->update( dt);
            break;
        }
        case AppState::MENU: {
            break;
        }
        case AppState::PAUSED: {
            break;
        }
        default: std::cerr << "Unknown state" << std::endl;
    }
}

void GameManager::handleUI(const float dt) {
    uiManager_.startFrame();
    switch (state_) {
        case AppState::MENU:   uiManager_.renderMainMenuOverlay(commands(), resources, aiCount_); break;
        case AppState::PLAYING: uiManager_.renderCheckPointOverlay(commands(), gameSnapShot_); break;
        case AppState::MAKER: uiManager_.renderMapMakerOverlay(commands(), mapMakerSnapShot_); break;
        case AppState::PAUSED: uiManager_.renderPauseMenuOverlay(commands()); break;
        default: std::cerr << "unknown state" << std::endl;
    }
    uiManager_.endFrame();
}

void GameManager::render(threepp::GLRenderer& renderer) const {
    if (gameScene_ && ( AppState::PLAYING == state_ || AppState::PAUSED == state_)) {
        gameScene_->render(renderer);
    } else if ( menuScene_ && AppState::MENU == state_ ) {
        menuScene_->render(renderer);
    } else if ( mapMakerScene_ && AppState::MAKER == state_ ) {
        mapMakerScene_->render(renderer);
    }
}


