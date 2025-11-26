//
// Created by Mella on 20.11.2025.
//

#ifndef CARSIM_GAMEMANAGER_HPP
#define CARSIM_GAMEMANAGER_HPP

#include <memory>
#include <string>
#include <GameLogic.hpp>
#include <GameScene.hpp>
#include <ResourceManager.hpp>
#include "MapCreatorScene.hpp"
#include "MenuScene.hpp"
#include "UIManager.hpp"

enum class AppState {
    MENU,
    PLAYING,
    PAUSED,
    MAKER
};

class GameManager {
    public:

        float globScale = 8.f;
        float startTimer_ = 0;
        int aiCount_ = 0;

        GameManager() : keyPressed_(threepp::KeyAdapter::Mode::KEY_PRESSED,
                                    [&](const threepp::KeyEvent evt) { onKeyPressed(evt); }),
                        keyReleased_(threepp::KeyAdapter::Mode::KEY_RELEASED,
                                     [&](const threepp::KeyEvent evt) { onKeyReleased(evt); }),
                        gameSnapshot_(), resources(std::make_unique<ResourceManager>())
        {
            startMenu();
        }

        void attachListeners(threepp::Canvas& canvas) {
            canvas.addKeyListener(keyPressed_);
            canvas.addKeyListener(keyReleased_);
        }

        void startMenu() {
            menuScene_ = std::make_unique<MenuScene>(*resources);
            state_ = AppState::MENU;
        }
        void startRace(const std::string& mapName, const std::string& modelName) {
            gameLogic = std::make_unique<GameLogic>(mapName, aiCount_);
            gameSnapshot_ = gameLogic->snapshot();
            if (gameLogic)  {
                gameScene_ = std::make_unique<GameScene>(
                    gameSnapshot_ , *resources,
                    modelName
                );
            } else {
                std::cout <<"no game"<< std::endl;
            }
            setStartTimer(2);
            state_ = AppState::PLAYING;
        }
        void startMapMaker() {
            mapCreatorLogic = std::make_unique<MapCreatorLogic>();
            //currentScene_ = std::make_unique<MapCreatorScene>(*mapCreatorLogic);
            state_ = AppState::MAKER;
        }


        void setStartTimer(const float startTimer) {
            startTimer_ = startTimer;
        }
        [[nodiscard]] int getAiCount() const { return aiCount_;}
        void setAiCount(const int aiCount) { aiCount_ = aiCount;}

        void pauseGame() {
            if (state_ == AppState::PLAYING) state_ = AppState::PAUSED;
        }
        void resumeGame() {
            if (state_ == AppState::PAUSED) state_ = AppState::PLAYING;
        }

        void onKeyPressed(const threepp::KeyEvent& evt) {
            if (const auto* pc = playerController()) pc->onKeyPressed(evt);
            if (evt.key == threepp::Key::P) {
                if (state_ == AppState::PLAYING) pauseGame();
                else if (state_ == AppState::PAUSED) resumeGame();
            }
        }
        void onKeyReleased(const threepp::KeyEvent& evt) const {
            if (const auto* pc = playerController()) pc->onKeyReleased(evt);
        }

        void update(const float dt) {
            if (state_ == AppState::PLAYING && gameLogic) {
                if (startTimer_ >= 0) startTimer_ -= dt;
                else {
                    gameLogic->update(dt);
                    gameSnapshot_ = gameLogic->snapshot();
                    gameScene_->update(gameSnapshot_, dt);
                }
            }
        }

        void handleUI(UIManager& ui, const float dt) {
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
                case AppState::PLAYING: {
                    const auto& car = gameSnapshot_.cars[0];
                    UIManager::renderOverlay(car.speed, car.boostFactor, car.position, "map");
                    ui.renderCheckPointOverlay(gameSnapshot_, dt);
                    if (startTimer_ > 0.f) UIManager::renderStartTimer(startTimer_, dt);
                    break;
                }
                case AppState::MAKER: {
                    const auto [save, undo, clear, goback, looped, newMapSize] = UIManager::renderMakerMenu(
                        mapCreatorLogic->getPoints(),
                        mapCreatorLogic->getMapSize(),
                        mapCreatorLogic->isLooped()
                    );
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
            }
            UIManager::endFrame();
        }

        void render(threepp::GLRenderer& renderer) const {
            if (state_ == AppState::PLAYING && gameScene_) gameScene_->render(renderer);
            if (state_ == AppState::MENU && menuScene_) menuScene_->render(renderer);
        }

        [[nodiscard]] PlayerController* playerController() const {
            if (!gameLogic || gameSnapshot_.cars.empty()) return nullptr;
            return dynamic_cast<PlayerController*>(gameLogic->getPlayerCar().controller.get());
        }

    private:
        threepp::KeyAdapter keyPressed_;
        threepp::KeyAdapter keyReleased_;
        GameState gameSnapshot_;
        AppState state_ = AppState::MENU;
        std::unique_ptr<ResourceManager> resources;
        std::unique_ptr<GameLogic> gameLogic;
        std::unique_ptr<GameScene> gameScene_;
        std::unique_ptr<MenuScene> menuScene_;
        std::unique_ptr<MapCreatorLogic> mapCreatorLogic;

};

#endif // CARSIM_GAMEMANAGER_HPP
