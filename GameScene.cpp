//
// Created by Mella on 18.10.2025.
//

#include "GameScene.hpp"

#include <map>

GameScene::GameScene(threepp::Vector3& carPos, float& carRotation, RaceGame& raceGame) {
        scene = threepp::Scene::create();

        // camera
        camera = threepp::PerspectiveCamera::create(60, 16.0f/9.0f, 0.1f, 10000);

        // loaders
        threepp::OBJLoader ol;
        threepp::TextureLoader tl;

        auto light = threepp::DirectionalLight::create(threepp::Color::white, 1.0f);
        light->position.set(10, 10, 10);
        scene->add(light);

        auto ambient = threepp::AmbientLight::create(threepp::Color::white, 0.5f);
        scene->add(ambient);

        auto mapTexture = tl.load("C://Users/Mella/Dev/CLionProjects/CarSimV0/resources/maps/newdawn.png");
        mapTexture->wrapS = mapTexture->wrapT = threepp::TextureWrapping::Repeat;

        auto mapWidth = 1000;  // scale as needed
        auto mapHeight = 1000; // scale as needed

        auto mapGeo = threepp::PlaneGeometry::create(mapWidth, mapHeight);
        auto mapMat = threepp::MeshBasicMaterial::create();
        mapMat->map = mapTexture;
        mapMat->side = threepp::Side::Double; // so you can see it from above

        auto mapMesh = threepp::Mesh::create(mapGeo, mapMat);
        mapMesh->rotation.x = -M_PI / 2;// lay it flat on XZ plane
        mapMesh->scale.z = -1.0f;
        mapMesh->position.set(mapWidth/2, 0, mapHeight/2);

        scene->add(mapMesh);


        carMesh = ol.load("C://Users/Mella/Dev/CLionProjects/CarSimV0/resources/models/Car.obj" , true);
        if (!carMesh) {
            std::cerr << "Failed to load wheel.obj!\n";
            return;
        }

        carMesh->position = carPos;
        carMesh->rotation.y = carRotation;
        scene->add(carMesh);

    }
void GameScene::update(const threepp::Vector3& carPos, const float& carRotation, const float& dt) const
{
    carMesh->position = carPos;
    carMesh->rotation.y = carRotation;
    constexpr float camDistance = 10.0f;
    constexpr float camHeight = 5.0f;

    const float rot = carRotation;
    const auto forwardDir = threepp::Vector3(std::sin(rot), 0, std::cos(rot));
    const threepp::Vector3 camOffset = forwardDir * camDistance;
    threepp::Vector3 camPos = carPos - camOffset;

    camPos.y += camHeight;

    constexpr float smoothingSpeed = 10.0f;
    camera->position.lerp(camPos, 1.0f - std::exp(-smoothingSpeed * dt));
    camera->lookAt(carPos);
}
