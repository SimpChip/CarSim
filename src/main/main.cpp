
#include <threepp/threepp.hpp>
#include <CarLogic.hpp>
#include <GameScene.hpp>
#include <GameLogic.hpp>
#include <GameManager.hpp>
#include <ctime>
#include <cmath>
#include <UIManager.hpp>

#define STB_IMAGE_IMPLEMENTATION


int main() {
    float globScale = 10.f;
    threepp::Canvas canvas("Race Instanced");
    threepp::GLRenderer renderer(canvas.size());
    renderer.setClearColor(threepp::Color::whitesmoke);

    GameManager manager;
    UIManager ui(canvas.windowPtr());


    canvas.onWindowResize([&](const threepp::WindowSize& size) {
        if (manager.gameScene) {
            manager.gameScene->camera->aspect = size.aspect();
            manager.gameScene->camera->updateProjectionMatrix();
        }
            renderer.setSize(size);
    });

    threepp::KeyAdapter keyPressed(
        threepp::KeyAdapter::Mode::KEY_PRESSED,
        [&](threepp::KeyEvent evt){
            if (auto* pc = manager.playerController()) pc->onKeyPressed(evt);
            // Global keys
            if (evt.key == threepp::Key::P ) {
                if (manager.state == AppState::PLAYING) manager.pauseGame();
                else if (manager.state == AppState::PAUSED) manager.resumeGame();
            }
        });

    threepp::KeyAdapter keyReleased(
        threepp::KeyAdapter::Mode::KEY_RELEASED,
        [&](threepp::KeyEvent evt){
            if (auto* pc = manager.playerController()) pc->onKeyReleased(evt);
        });

    canvas.addKeyListener(keyPressed);
    canvas.addKeyListener(keyReleased);

    std::cout << "Started Game, Have fun!"<< std::endl;
    threepp::Clock clock;
    canvas.animate([&](){
       const float dt = clock.getDelta();

       switch (manager.state) {
           case AppState::MENU: {
               UIManager::startFrame();
               std::string selectedMap;
               std::string selectedModel;
               if (ui.renderMainMenu(selectedMap, selectedModel)) {
                   manager.startRace(selectedMap, selectedModel);
               }
               UIManager::endFrame();
               break;
           }

           case AppState::PLAYING: {
               manager.update(dt);

               if (manager.gameScene && manager.gameScene->camera && manager.gameScene->scene_) {
                   renderer.render(*manager.gameScene->scene_, *manager.gameScene->camera);
               }

               UIManager::startFrame();
               GameLogic &logic = *manager.gameLogic;
               const auto& car = logic.getCarLogic(0);
               UIManager::renderOverlay(car.getSpeed(), car.getBoostFactor(), car.getPosition(), "map");
               ui.renderCheckPointOverlay(logic, dt);
               const float startTimer = manager.startTimer_;
               if (startTimer > 0.f) {
                   UIManager::renderStartTimer(startTimer, dt);
               }
               UIManager::endFrame();
               break;
           }

           case AppState::PAUSED: {
               if (manager.gameScene && manager.gameScene->camera && manager.gameScene->scene_) {
                   renderer.render(*manager.gameScene->scene_, *manager.gameScene->camera);
               }

               UIManager::startFrame();
                   auto [resume, quitToMenu] = ui.renderPauseMenu();
                   if (resume) {
                       manager.resumeGame();
                   }
                   if (quitToMenu) {
                       manager.returnToMenu();
                   }
               UIManager::endFrame();
               break;
           }
       }
   });

    return 0;
}