

#include "core/GameManager.hpp"

GameManager::GameManager()
    : keyPressed_(threepp::KeyAdapter::Mode::KEY_PRESSED,
                  [&](const threepp::KeyEvent evt) { onKeyPressed(evt); }),
      keyReleased_(threepp::KeyAdapter::Mode::KEY_RELEASED,
                   [&](const threepp::KeyEvent evt) { onKeyReleased(evt); }),
      gameSnapshot_(),
      resources(std::make_unique<ResourceManager>()) {
    startMenu();
}

void GameManager::attachListeners(threepp::Canvas& canvas) {
    canvas.addKeyListener(keyPressed_);
    canvas.addKeyListener(keyReleased_);
}

void GameManager::startMenu() {
    menuScene_ = std::make_unique<MenuScene>(*resources);
    state_ = AppState::MENU;
}

void GameManager::startRace(const std::string& mapName, const std::string& modelName) {
    gameLogic.reset();
    gameScene_.reset();
    gameLogic = std::make_unique<GameLogic>(mapName, aiCount_);
    if (gameLogic) {
        gameLogic->update(0);
        gameSnapshot_ = gameLogic->snapshot();
        gameScene_ = std::make_unique<GameScene>(gameSnapshot_, *resources, modelName);
    } else {
        std::cout << "no game" << std::endl;
    }
    state_ = AppState::STARTING;
    setStartTimer(6);
}

void GameManager::startMapMaker() {
    mapCreatorLogic = std::make_unique<MapCreatorLogic>();
    state_ = AppState::MAKER;
}

void GameManager::setStartTimer(const float startTimer) { startTimer_ = startTimer; }
int GameManager::getAiCount() const { return aiCount_; }
void GameManager::setAiCount(const int aiCount) { aiCount_ = aiCount; }

void GameManager::pauseGame() { if (state_ == AppState::PLAYING) state_ = AppState::PAUSED; }
void GameManager::resumeGame() { if (state_ == AppState::PAUSED) state_ = AppState::PLAYING; }

void GameManager::onKeyPressed(const threepp::KeyEvent& evt) {
    if (const auto* pc = playerController()) pc->onKeyPressed(evt);
    if (evt.key == threepp::Key::P) {
        if (state_ == AppState::PLAYING) pauseGame();
        else if (state_ == AppState::PAUSED) resumeGame();
    }
}

void GameManager::onKeyReleased(const threepp::KeyEvent& evt) const {
    if (const auto* pc = playerController()) pc->onKeyReleased(evt);
}

void GameManager::update(const float dt) {

    if (state_ == AppState::PLAYING && gameLogic && gameScene_) {
        gameLogic->update(dt);
        gameSnapshot_ = gameLogic->snapshot();
        gameScene_->update(gameSnapshot_, dt);
    } else if (state_ == AppState::STARTING && gameLogic && gameScene_) {
        gameScene_->update(gameSnapshot_, dt);
        if (startTimer_ >= 0) {
            startTimer_ -= dt;
        }
        else {
            gameLogic->preUpdate(dt);
            state_ = AppState::PLAYING;
        }
    }
}

void GameManager::handleUI(UIManager& ui, const float dt) {
    UIManager::startFrame();
    switch (state_) {
        case AppState::MENU: {
            std::string selectedMap, selectedModel;
            auto [start, maker, aiCount] = UIManager::renderMainMenu(selectedMap, selectedModel, getAiCount());
            if (start) startRace(selectedMap, selectedModel);
            if (maker) startMapMaker();
            if (aiCount != aiCount_) aiCount_ = aiCount;
            break;
        }
        case AppState::STARTING:
        case AppState::PLAYING: {
            const auto& car = gameSnapshot_.cars[0];
            UIManager::renderOverlay(car.speed, car.boostFactor, car.position, "map");
            ui.renderCheckPointOverlay(gameSnapshot_, dt);
            if (startTimer_ > 0.f) UIManager::renderStartTimer(startTimer_, dt);
            break;
        }
        case AppState::MAKER: {
            const auto [save, undo, clear, goback, looped, newMapSize] =
                UIManager::renderMakerMenu(mapCreatorLogic->getPoints(),
                                           mapCreatorLogic->getMapSize(),
                                           mapCreatorLogic->isLooped());
            if (save) MapCreatorLogic::saveMap();
            if (undo) mapCreatorLogic->undoLastPoint();
            if (clear) mapCreatorLogic->clearPoints();
            if (goback) startMenu();
            if (newMapSize != mapCreatorLogic->getMapSize()) mapCreatorLogic->setMapSize(newMapSize);
            mapCreatorLogic->setLooped(looped);
            break;
        }
        case AppState::PAUSED: {
            auto [resume, quitToMenu] = UIManager::renderPauseMenu();
            if (resume) resumeGame();
            if (quitToMenu) startMenu();
            break;
        }
        default: ;
    }
    UIManager::endFrame();
}

void GameManager::render(threepp::GLRenderer& renderer) const {
    if (state_ == AppState::PLAYING || state_ == AppState::STARTING && gameScene_) gameScene_->render(renderer);
    if (state_ == AppState::MENU && menuScene_) menuScene_->render(renderer);
}

PlayerController* GameManager::playerController() const {
    if (!gameLogic || gameSnapshot_.cars.empty()) return nullptr;
    return dynamic_cast<PlayerController*>(gameLogic->getPlayerCar().controller.get());
}

