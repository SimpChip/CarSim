//
// Created by Mella on 18.10.2025.
//



#include <map>

#include "cmake-build-debug-mingw/_deps/threepp-src/src/threepp/renderers/gl/GLProperties.hpp"
#include <threepp/threepp.hpp>
#include <iostream>
#include <regex>
#include <MapLogic.hpp>

class GameScene
{
public:
    std::shared_ptr<threepp::Scene> scene_;
    std::shared_ptr<threepp::PerspectiveCamera> camera;
    std::shared_ptr<threepp::Group> carMesh;
    std::vector<std::vector<std::shared_ptr<threepp::Texture>>> textures;
    std::shared_ptr<threepp::Mesh> colBox;
    std::shared_ptr<threepp::PlaneGeometry> mapGeo;
    float scale_ = 10;


    GameScene(threepp::Vector3& carPos, float carRotation, MapLogic& raceGame, std::string mapName) {
        scene_ = threepp::Scene::create();
        // camera
        camera = threepp::PerspectiveCamera::create(60, 16.0f/9.0f, 0.1f, 10000);

        // loaders
        threepp::OBJLoader ol;
        threepp::TextureLoader tl;

        auto light = threepp::DirectionalLight::create(threepp::Color::white, 1.0f);
        light->position.set(10, 10, 10);
        scene_->add(light);

        auto ambient = threepp::AmbientLight::create(threepp::Color::white, 0.5f);
        scene_->add(ambient);

        loadMapParts(mapName, tl, 2,2);

        carMesh = ol.load(std::string(DATA_DIR) + "/models/Car.obj", true);
        if (!carMesh) {
            std::cerr << "Failed to load wheel.obj!\n";
            return;
        }
        carMesh->position = carPos;
        carMesh->rotation.y = carRotation;
        carMesh->scale = Vector3(scale_*3+1, scale_*3+1, scale_*3+1);
        scene_->add(carMesh);

    }

    void loadMapParts(const std::string& mapName, threepp::TextureLoader& tl, int rows, int cols) {

        const std::string baseDir = std::string(DATA_DIR) + "/maps/"+ mapName;

        std::cout << baseDir << std::endl;
        std::regex folderPattern(R"((\d+)_(\d+))"); // matches folders like 0_1, 2_3, etc.

        for (const auto& entry : std::filesystem::directory_iterator(baseDir)) {
            if (!entry.is_directory()) continue;

            std::string folderName = entry.path().filename().string();
            std::smatch match;
            if (!std::regex_match(folderName, match, folderPattern)) {
                continue;
            }

            const int x = std::stoi(match[1]);
            const int y = std::stoi(match[2]);

            std::string texturePath = baseDir + "/" + folderName + "/m.png";
            if (!std::filesystem::exists(texturePath)) {
                std::cerr << "Warning: Missing texture file in folder: " << texturePath << std::endl;
                continue;
            }
            auto texture = tl.load(texturePath);
            auto mesh = createMapPart(x, y, texture);
            scene_->add(mesh);
        }
        std::cout << "GameScene is created" << std::endl;
    }
    void update(const threepp::Vector3& carPos, float carRotation, const float& dt) const
    {
        carMesh->position = carPos;
        carMesh->rotation.y = carRotation;
        float camDistance = 30*scale_ * 3+3;
        float camHeight = 6*scale_ * 3 +3;

        float rot = carRotation;
        const auto forwardDir = threepp::Vector3(std::sin(rot), 0, std::cos(rot));
        const threepp::Vector3 camOffset = forwardDir * camDistance;
        threepp::Vector3 camPos = carPos - camOffset;

        camPos.y += camHeight;

        constexpr float smoothingSpeed = 10.0f;
        camera->position.lerp(camPos, 1.0f - std::exp(-smoothingSpeed * dt));
        camera->lookAt(carPos);
    }


    std::shared_ptr<threepp::Mesh> createMapPart(int Mx,int Mz,std::shared_ptr<threepp::Texture> texture)
    {
        int mapWidth = texture->image().width;
        int mapHeight = texture->image().height;
        auto mapMat  = threepp::MeshBasicMaterial::create();
        mapGeo = threepp::PlaneGeometry::create(mapWidth, mapHeight );
        texture->wrapS = texture->wrapT = threepp::TextureWrapping::Repeat;
        texture->magFilter = threepp::Filter::Linear;// sharp pixel edges
        texture->minFilter = threepp::Filter::Linear;



        mapGeo = threepp::PlaneGeometry::create(mapWidth, mapHeight);
        mapMat->map = texture;
        mapMat->side = threepp::Side::Double;

        auto mapMesh = threepp::Mesh::create(mapGeo, mapMat);
        mapMesh->rotation.x = -M_PI / 2;// lay it flat on XZ plane
        mapMesh->scale.z = -1.0f;
        mapMesh->position.set(mapWidth/2 + mapWidth * Mx , 0, mapHeight/2 + mapHeight * Mz);
        mapMat->transparent = true;

        mapMat->transparent = true;
        mapMat->opacity = 1.f;
        mapMat->alphaTest = 0.2f;
        mapMat->blending = threepp::Blending::Normal;

        return mapMesh;
    }
};