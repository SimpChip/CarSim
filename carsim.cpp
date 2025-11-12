
#include <threepp/threepp.hpp>
#include <CarLogic.hpp>
#include <MapLogic.hpp>
#include <GameScene.cpp>
#include <ctime>
#include <cmath>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


#define STB_IMAGE_IMPLEMENTATION


int main() {


    float globScale = 10.f;
    threepp::Canvas canvas("Race Instanced");
    threepp::GLRenderer renderer(canvas.size());
    renderer.setClearColor(threepp::Color::whitesmoke);
    std::string mapName = "anotherTwo";
    MapLogic mapLogic(mapName);
    auto [carStartPos, carStartRot] = mapLogic.getStartPositionAndRotation();
    CarLogic carLogic(carStartPos, carStartRot, globScale );
    GameScene gameScene(mapName, globScale, mapLogic, carLogic);

    canvas.onWindowResize([&](const threepp::WindowSize& size) {
        gameScene.camera->aspect = size.aspect();
        gameScene.camera->updateProjectionMatrix();
        renderer.setSize(size);
    });

    threepp::KeyAdapter adapter(threepp::KeyAdapter::Mode::KEY_PRESSED, [&](threepp::KeyEvent evt){
        switch(evt.key) {
            case threepp::Key::UP: gameScene.setCameraDistanceInput(1); break;
            case threepp::Key::DOWN: gameScene.setCameraDistanceInput(-1); break;
            case threepp::Key::Q: carLogic.setBoostFactor(2);
            case threepp::Key::W: carLogic.setAccelerationInput(1.0f); break;
            case threepp::Key::S: carLogic.setAccelerationInput(-1.0f); break;
            case threepp::Key::A: carLogic.setSteeringLeftActive(true); break;
            case threepp::Key::D: carLogic.setSteeringRightActive(true); break;
        default: ;
        }
    });

    threepp::KeyAdapter adapterRelease(threepp::KeyAdapter::Mode::KEY_RELEASED, [&](threepp::KeyEvent evt){
        switch(evt.key) {
            case threepp::Key::UP:
            case threepp::Key::DOWN: gameScene.setCameraDistanceInput(0); break;
            case threepp::Key::W:
            case threepp::Key::S: carLogic.setAccelerationInput(0.0f); break;
            case threepp::Key::A: carLogic.setSteeringLeftActive(false); break;
            case threepp::Key::D: carLogic.setSteeringRightActive(false); break;
        default: ;
        }
    });

    canvas.addKeyListener(adapter);
    canvas.addKeyListener(adapterRelease);

    std::cout << "Started Game, Have fun!"<< std::endl;
    threepp::Clock clock;
    canvas.animate([&](){
        float dt = clock.getDelta();

        carLogic.update(dt);
        mapLogic.handleMapCollisions(carLogic, dt);
        mapLogic.handlePickupCollisions(carLogic, dt);
        mapLogic.handleCheckPointing(carLogic,dt);
        gameScene.update(carLogic, dt);

        renderer.render(*gameScene.scene_, *gameScene.camera);
    });

    return 0;
}


