//
// Created by Mella on 18.10.2025.
//

#include "GameScene.hpp"

#include <map>

GameScene::GameScene(threepp::Vector3& carPos, float& carRotation, RaceGame& raceGame) {
        auto map = raceGame.raceMap;
        auto mapWidth = map[0].size();
        auto mapHeight = map.size();
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

        // ground
        auto grassGeo = threepp::PlaneGeometry::create(mapWidth * 10, mapHeight * 10);
        auto grassMat = threepp::MeshBasicMaterial::create();
        grassMat->color = threepp::Color::green;
        grassMat->side = threepp::Side::Double;
        auto grass = threepp::Mesh::create(grassGeo, grassMat);
        grass->rotation.x = -M_PI/2;
        grass->position.set(mapWidth/2 - 0.5f, -0.5f, mapHeight/2 - 0.5f);
        scene->add(grass);


        // palm
        // auto palmObj = ol.load("C://Users/Mella/Dev/CLionProjects/CarSimV0/resources/models/palm_01.obj" , true);
        // if (!palmObj) {
        //     std::cerr << "Failed to load palm!\n";
        //     return;
        // }
        //
        // threepp::Mesh* palmMesh = nullptr;
        // palmObj->traverse([&](threepp::Object3D& obj) {
        //     if (!palmMesh) palmMesh = dynamic_cast<threepp::Mesh*>(&obj);
        // });

        // if (!palmMesh) {
        //     std::cerr << "Palm OBJ has no mesh!\n";
        //     return;
        // }

        // auto palmGeo = palmMesh->geometry();
        // auto palmMat = threepp::MeshStandardMaterial::create();



        // map tiles
        auto roadTexture = tl.load("C://Users/Mella/Dev/CLionProjects/CarSimV0/resources/textures/road.png");
        roadTexture->wrapS = roadTexture->wrapT = threepp::TextureWrapping::Repeat; // repeat texture
        roadTexture->repeat.set(1, 1);

        auto sandTexture = tl.load("C://Users/Mella/Dev/CLionProjects/CarSimV0/resources/textures/sand.png");
        sandTexture->wrapS = sandTexture->wrapT = threepp::TextureWrapping::Repeat; // repeat texture
        sandTexture->repeat.set(1, 1);

        auto wallTexture = tl.load("C://Users/Mella/Dev/CLionProjects/CarSimV0/resources/textures/sand.png");
        wallTexture->wrapS = wallTexture->wrapT = threepp::TextureWrapping::Repeat; // repeat texture
        wallTexture->repeat.set(1, 1);

        auto floorGeo = threepp::BoxGeometry::create(1.0f, 1.0f, 1.0f);
        auto wallGeo = threepp::CylinderGeometry::create(0.75f, 0.75f, 20.0f);

        auto wallMat = threepp::MeshBasicMaterial::create(); wallMat->color = threepp::Color::gray;
        wallMat->map = wallTexture;
        wallMat->side = threepp::Side::Double;
        auto roadMat = threepp::MeshBasicMaterial::create(); roadMat->color = threepp::Color::lightsalmon;
        roadMat->map = roadTexture;
        roadMat->side = threepp::Side::Double;
        auto sandMat = threepp::MeshBasicMaterial::create(); sandMat->color = threepp::Color::beige;
        sandMat->map = sandTexture;
        sandMat->side = threepp::Side::Double;

        int roadCount=0, sandCount=0, wallCount=0, palmCount=0;
        for (auto& row: map) for (auto tile: row)
        {
            if(tile==1) roadCount++;
            else if(tile==3) sandCount++;
            else if(tile==2) wallCount++;
            else if (tile==4) {
                palmCount++; sandCount++;
        }
        }

       // auto palmInst = threepp::InstancedMesh::create(palmGeo, palmMat, palmCount);
        auto roadInst = threepp::InstancedMesh::create(floorGeo, roadMat, roadCount);
        auto sandInst = threepp::InstancedMesh::create(floorGeo, sandMat, sandCount);
        auto wallInst = threepp::InstancedMesh::create(wallGeo, wallMat, wallCount);


        int r=0, s=0, w=0, p=0; threepp::Matrix4 matrix;
        for(int z=0; z<map.size(); ++z){
            for(int x=0; x<map[0].size(); ++x){
                matrix.identity();
                matrix.setPosition(static_cast<float>(x), (map[z][x]==2?0.f:-0.9f), static_cast<float>(z));
                if(map[z][x]==1) roadInst->setMatrixAt(r++, matrix);
                else if(map[z][x]==3) sandInst->setMatrixAt(s++, matrix);
                else if(map[z][x]==2) wallInst->setMatrixAt(w++, matrix);
                else if(map[z][x]==4) { sandInst->setMatrixAt(s++, matrix); };
            }
        }

        scene->add(roadInst);
        scene->add(sandInst);
        scene->add(wallInst);
        //scene->add(palmInst);

        // finishline
        auto finishLine = raceGame.checkpointMap.at(255);
        const auto& p0 = finishLine.points[0];
        const auto& p1 = finishLine.points[1];

        // Compute center position
        Vector3 center = finishLine.midpoint();

        // Compute size
        float length = std::sqrt(
            (p1.x - p0.x) * (p1.x - p0.x) +
            (p1.z - p0.z) * (p1.z - p0.z)
        );

        float width = 6.f; // thickness of the finish line

        // Create a white rectangle (PlaneGeometry)
        auto finishGeo = threepp::PlaneGeometry::create(length, width);
        auto finishLineTexture = tl.load("C://Users/Mella/Dev/CLionProjects/CarSimV0/resources/textures/finishline.jpg");
        auto finishMat = threepp::MeshBasicMaterial::create();
        finishMat->map = finishLineTexture;
        finishMat->side = threepp::Side::Double;

        auto finishMesh = threepp::Mesh::create(finishGeo, finishMat);

        // Rotate to align with line if necessary
        float angle = finishLine.angle();
        finishMesh->rotation.y = -angle; // negative because of threepp coords

        finishMesh->position.set(center.x, 16, center.z);

        scene->add(finishMesh);

        // carMesh
        carMesh = ol.load("C://Users/Mella/Dev/CLionProjects/CarSimV0/resources/models/Car.obj" , true);
        if (!carMesh) {
            std::cerr << "Failed to load wheel.obj!\n";
            return;
        }

        carMesh->position = carPos;
        carMesh->rotation.y = carRotation;
        scene->add(carMesh);

        auto colGeo = threepp::BoxGeometry::create(2, 2, 4);
        auto colMat = threepp::MeshBasicMaterial::create(); colMat->color = threepp::Color::white;
        //colBox = threepp::Mesh::create(colGeo, colMat);
        //scene->add(colBox);
    }
void GameScene::update(const threepp::Vector3& carPos, const float& carRotation, const float& dt) const
{
    carMesh->position = carPos;
    //colBox->position = carPos;
    carMesh->rotation.y = carRotation;
    //colBox->rotation.y = carRotation;

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
