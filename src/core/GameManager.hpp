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

enum class AppState {
    MENU,
    PLAYING,
    PAUSED
};

class GameManager {
    public:
        ResourceManager resources;
        AppState state = AppState::MENU;

        std::unique_ptr<GameLogic> gameLogic;
        std::unique_ptr<GameScene> gameScene;

        float globScale = 8.f;
        float startTimer_ = 0;

        GameManager() = default;

        void startRace(const std::string& mapName, const std::string& modelName) {

            startTimer_ = 3;

            gameLogic.reset();
            gameLogic = std::make_unique<GameLogic>(mapName, 32);

            gameScene.reset();
            if (gameScene) {
                std::cout << "it exists" << std::endl;}

            gameScene = std::make_unique<GameScene>(
                mapName, modelName, globScale,
                gameLogic->getCars(),
                gameLogic->getMapLogic(),
                gameLogic->getCpLogic(),
                gameLogic->getPickupLogic(),
                gameLogic->getCar(0).logic.getPosition(),
                resources
            );



            state = AppState::PLAYING;
        }

        void returnToMenu() {
            gameLogic.reset();
            gameScene.reset();
            state = AppState::MENU;
        }

        void pauseGame() {
            if (state == AppState::PLAYING) state = AppState::PAUSED;
        }

        void resumeGame() {
            if (state == AppState::PAUSED) state = AppState::PLAYING;
        }

        void update(const float dt) {
            if (state == AppState::PLAYING && gameLogic && gameScene) {
                if (startTimer_ > 0.f ) {
                    startTimer_ -= dt;
                } else {
                    gameLogic->update(dt);
                }
                gameScene->update(gameLogic->getCars(), gameLogic->getPickupLogic(), dt);
            }
        }

        PlayerController* playerController() const {
            if (!gameLogic || gameLogic->getCars().empty()) return nullptr;
            return dynamic_cast<PlayerController*>(gameLogic->getCar(0).controller.get());
        }
};

#endif // CARSIM_GAMEMANAGER_HPP
