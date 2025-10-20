
#include <threepp/threepp.hpp>
#include <CarLogic.hpp>
#include <RaceGame.hpp>
#include <GameScene.hpp>
#include <ctime>
#include <cmath>

void handleMapCollisions(CarLogic& car, const std::vector<std::vector<int>>& map, const float sandDrag, const float carLength, const float carWidth)
{
    const Vector3 prevPos = car.getPreviousPosition();
    const Vector3 pos = car.getPosition();
    const Vector3 movement = pos - prevPos;

    const float movementDist = movement.length();
    if (movementDist < 0.0001f) return;

    const Vector3 forward(std::sin(car.getRotation()), 0, std::cos(car.getRotation()));
    const Vector3 right(forward.z, 0, -forward.x);
    const int samples = static_cast<int>(movementDist * 10.0f) + 1;
    const float halfWidth = carWidth / 2.0f;

    bool sand = false;

    for (int i = 1; i <= samples; ++i)
    {
        Vector3 stepPos = prevPos + movement * (static_cast<float>(i) / samples);

        for (float w = -halfWidth; w <= halfWidth; w += 0.5f)
        {
            const Vector3 checkPoint = stepPos + forward * (carLength / 2.0f) + right * w;
            const int tileX = std::clamp(static_cast<int>(checkPoint.x), 0, static_cast<int>(map[0].size() - 1));
            const int tileZ = std::clamp(static_cast<int>(checkPoint.z), 0, static_cast<int>(map.size() - 1));
            const int tile = map[tileZ][tileX];

            if (tile == 0 || tile == 2) // Vegg eller ingenting
            {
                const float backDist = static_cast<float>(i - 1) / samples;
                const Vector3 newPos = prevPos + movement * backDist;
                car.setPosition(newPos);
                car.stopCar();
                return;
            }
            if (tile == 3) // Sand
            {
                sand = true;
            }
        }
    }

    if (sand)
        car.applyBraking(sandDrag);
}

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

    RaceGame race("newdawn");
    auto [startPos, startRot] = race.getStartPositionAndRotation();
    CarLogic carLogic(startPos, startRot);


    GameScene gameScene(carLogic.getPosition(),carLogic.getRotation(), race);

    canvas.onWindowResize([&](const threepp::WindowSize& size) {
        gameScene.camera->aspect = size.aspect();
        gameScene.camera->updateProjectionMatrix();
        renderer.setSize(size);
    });

    threepp::KeyAdapter adapter(threepp::KeyAdapter::Mode::KEY_PRESSED, [&](threepp::KeyEvent evt){
        switch(evt.key) {
            case threepp::Key::W: carLogic.setAccelerationInput(1.0f); break;
            case threepp::Key::S: carLogic.setAccelerationInput(-1.0f); break;
            case threepp::Key::A: carLogic.setSteeringInput(1.0f); break;
            case threepp::Key::D: carLogic.setSteeringInput(-1.0f); break;
            case threepp::Key::Q: carLogic.applyBoost(1.5); break;
        default: ;
        }
    });

    threepp::KeyAdapter adapterRelease(threepp::KeyAdapter::Mode::KEY_RELEASED, [&](threepp::KeyEvent evt){
        switch(evt.key) {
        case threepp::Key::W: carLogic.setAccelerationInput(0.0f); break;
        case threepp::Key::S: carLogic.setAccelerationInput(0.0f); break;
        case threepp::Key::A: carLogic.setSteeringInput(0.0f); break;
        case threepp::Key::D: carLogic.setSteeringInput(0.0f); break;
        default: ;
        }
    });

    canvas.addKeyListener(adapter);
    canvas.addKeyListener(adapterRelease);

    threepp::Clock clock;

    canvas.animate([&](){
        float dt = clock.getDelta();

        carLogic.update(dt);
        race.handleMapCollisions(carLogic);
        //handleMapCollisions(carLogic, race.raceMap, 0.8, 1, 1);

        const Vector3 prevPos = carLogic.getPreviousPosition();
        const Vector3 pos = carLogic.getPosition();


        gameScene.update(carLogic.getPosition(), carLogic.getRotation(), dt);

        renderer.render(*gameScene.scene, *gameScene.camera);
    });

    return 0;
}


