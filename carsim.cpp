
#include <threepp/threepp.hpp>
#include <CarLogic.hpp>
#include <MapLogic.hpp>
#include <GameScene.cpp>
#include <ctime>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION


bool isCrossingFinishLine(const Vector3& prevPos, const Vector3& pos,
                          const Checkpoint& finishLine)
{

    const Vector3& p0 = finishLine.points[0];
    const Vector3& p1 = finishLine.points[1];

    // Determine line bounds
    float minX = std::min(p0.x, p1.x);
    float maxX = std::max(p0.x, p1.x);
    float minZ = std::min(p0.z, p1.z);
    float maxZ = std::max(p0.z, p1.z);

    // Horizontal line
    if (std::abs(p0.z - p1.z) < 0.01f) {
        // Check if car is within the walls along X
        if (pos.x < minX || pos.x > maxX) return false;
        return (prevPos.z - p0.z) * (pos.z - p0.z) <= 0.0f;
    }

    // Vertical line
    if (std::abs(p0.x - p1.x) < 0.01f) {
        // Check if car is within the walls along Z
        if (pos.z < minZ || pos.z > maxZ) return false;
        return (prevPos.x - p0.x) * (pos.x - p0.x) <= 0.0f;
    }

    // Diagonal line
    Vector3 lineDir = p1 - p0;
    Vector3 toPrev = prevPos - p0;
    Vector3 toCurr = pos - p0;
    float cross1 = lineDir.x * toPrev.z - lineDir.z * toPrev.x;
    float cross2 = lineDir.x * toCurr.z - lineDir.z * toCurr.x;

    // Check perpendicular bounds along both axes
    if ((pos.x < minX || pos.x > maxX) || (pos.z < minZ || pos.z > maxZ))
        return false;

    return cross1 * cross2 <= 0.0f;
}



int main() {
    threepp::Canvas canvas("Race Instanced");
    threepp::GLRenderer renderer(canvas.size());
    renderer.setClearColor(threepp::Color(0.5f,0.8f,1.f));
    std::string mapName = "trackdesign";
    MapLogic race(mapName);
    auto [carStartPos, carStartRot] = race.getStartPositionAndRotation();
    CarLogic carLogic(carStartPos, carStartRot);


    GameScene gameScene(carStartPos, carStartRot, race, mapName);

    canvas.onWindowResize([&](const threepp::WindowSize& size) {
        gameScene.camera->aspect = size.aspect();
        gameScene.camera->updateProjectionMatrix();
        renderer.setSize(size);
    });

    threepp::KeyAdapter adapter(threepp::KeyAdapter::Mode::KEY_PRESSED, [&](threepp::KeyEvent evt){
        switch(evt.key) {
            case threepp::Key::Q: carLogic.applyBoost(100);
            case threepp::Key::W: carLogic.setAccelerationInput(1.0f); break;
            case threepp::Key::S: carLogic.setAccelerationInput(-1.0f); break;
            case threepp::Key::A: carLogic.setSteeringInput(1.0f); break;
            case threepp::Key::D: carLogic.setSteeringInput(-1.0f); break;
        default: ;
        }
    });

    threepp::KeyAdapter adapterRelease(threepp::KeyAdapter::Mode::KEY_RELEASED, [&](threepp::KeyEvent evt){
        switch(evt.key) {
        case threepp::Key::W:
        case threepp::Key::S: carLogic.setAccelerationInput(0.0f); break;
        case threepp::Key::A:
        case threepp::Key::D: carLogic.setSteeringInput(0.0f); break;
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
        race.handleMapCollisions(carLogic, dt);;
        gameScene.update(carLogic.getPosition(), carLogic.getRotation(), dt);

        renderer.render(*gameScene.scene_, *gameScene.camera);
    });

    return 0;
}


