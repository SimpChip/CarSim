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
    std::shared_ptr<threepp::Group> carBodyMesh;
    std::shared_ptr<threepp::Group> wheelFrontLMesh, wheelFrontRMesh, wheelBackLMesh, wheelBackRMesh;
    std::vector<std::shared_ptr<threepp::Group>> wheels;
    std::vector<std::vector<std::shared_ptr<threepp::Texture>>> textures;
    std::shared_ptr<threepp::Mesh> colBox;
    std::shared_ptr<threepp::PlaneGeometry> mapGeo;
    float scale_ = 10.f;
    float cameraUpdateInput = 0.f;
    float cameraDistance_ = 1.f;
    float cameraHeight_ = 1.f;

    GameScene(threepp::Vector3& carPos, float carRotation, MapLogic& raceGame, std::string mapName) {
        scene_ = threepp::Scene::create();
        // camera
        camera = threepp::PerspectiveCamera::create(60, 16.0f/9.0f, 1.f, 100000);
        camera->position = carPos - Vector3(0.f, 4.f, -5.f);
        camera->lookAt(carPos);
        // loadersa
        threepp::OBJLoader ol;
        threepp::TextureLoader tl;

        auto light = threepp::DirectionalLight::create(threepp::Color::white, 1.0f);
        light->position.set(10, 10, 10);
        scene_->add(light);

        auto ambient = threepp::AmbientLight::create(threepp::Color::white, 0.5f);
        scene_->add(ambient);

        loadMapParts(mapName, tl, 2,2);

        std::string obj_name = "cooltruck";
        const std::string base = std::string(DATA_DIR) + "/models/" + obj_name;
        const std::string carPath = base + "/car.obj";
        if (std::filesystem::exists(carPath)) {
            try {
                carMesh = ol.load(carPath, true);
                carMesh->scale = Vector3(scale_*1.f, scale_*1.f, scale_*1.f);
            } catch (...) {
                carMesh = createPlaceholder(scale_, 2.f, 1.f, 4.f);
            }
        } else {
            carMesh = createPlaceholder(scale_, 2.f, 1.f, 4.f);
        }
        carMesh->position = carPos;

        //crazycar = 1.05f, 0.4f,  1.4f
        //camping =  1 , 0.5, 1.05
        //redcar = 0.9 , 0.4, 1.3

        struct WheelDef { std::string filename; threepp::Vector3 pos; std::shared_ptr<threepp::Group>* target; };
        std::vector<WheelDef> wheelDefs = {
            { "wheel_fl.obj", threepp::Vector3( 1.f, 0.5f,  1.05f), &wheelFrontLMesh },
            { "wheel_fr.obj", threepp::Vector3(-1.f, 0.5f,  1.05f), &wheelFrontRMesh },
            { "wheel_bl.obj", threepp::Vector3( 1.f, 0.5f, -1.05f), &wheelBackLMesh  },
            { "wheel_br.obj", threepp::Vector3(-1.f, 0.5f, -1.05f), &wheelBackRMesh  },
        };

        wheels.clear(); // member vector
        for (auto &wd : wheelDefs) {
            const std::string wpath = base + "/" + wd.filename;
            std::shared_ptr<threepp::Group> loadedWheel = nullptr;

            if (std::filesystem::exists(wpath)) {
                try {
                    loadedWheel = ol.load(wpath, true); // ol.load accepts std::string
                } catch (...) {
                    loadedWheel = createPlaceholder(scale_, 0.5f,0.5f,0.5f);
                }
            } else {
                std::cout << wd.filename << "is not implmented"<<std::endl;
            }
            if (loadedWheel) {
                auto steeringPivot = threepp::Group::create();
                steeringPivot->position = wd.pos;
                steeringPivot->add(loadedWheel);
                carMesh->add(steeringPivot);
                *(wd.target) = steeringPivot;
                wheels.push_back(steeringPivot);
            } else {
                *(wd.target) = nullptr;
            }
        }

        scene_->add(carMesh);
    }
    void setCameraDistanceInput(float amount) { cameraUpdateInput = std::clamp(amount, -1.0f, 1.0f); }
    void updateCameraDistance(float dt) {
        cameraDistance_+= 1 * cameraUpdateInput * dt ;
        cameraHeight_+= 0.5 * cameraUpdateInput * dt;
    }

    static std::shared_ptr<threepp::Group> createPlaceholder(float scale, float x, float y, float z)
    {
        auto group = threepp::Group::create();
        auto geom = threepp::BoxGeometry::create(x * scale, y * scale, z * scale);
        auto mat  = threepp::MeshStandardMaterial::create();
        mat->color = threepp::Color::rebeccapurple;
        auto mesh = threepp::Mesh::create(geom, mat);
        mesh->position.set(0, 0.5f * scale, 0);
        group->add(mesh);
        return group;
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


    void update(CarLogic& carLogic, const float& dt)
    {
        carMesh->position = carLogic.getPosition();
        carMesh->rotation.y = carLogic.getRotation();

        updateCameraDistance(dt);

        auto applySteer = [&](const std::shared_ptr<threepp::Group> &wheel, float steerAngle) {
            if (!wheel) return;
            wheel->rotation.y = steerAngle;
        };
        float wheelRadius = scale_;
        float rollAmount = carLogic.velocity_.length() * dt / wheelRadius;
        auto applyWheelRoll = [&](const std::shared_ptr<threepp::Group> &wheelPivot, float sign = 1.0f) {
            if (!wheelPivot) return;
            if (wheelPivot->children.empty()) return;

            auto wheelMesh = wheelPivot->children.front();
            wheelMesh->rotation.x += rollAmount * sign;
        };

        float steeringYawFactor = 0.5f;
        float steer = carLogic.getCurrentSteeringInput() * steeringYawFactor;
        applySteer(wheelFrontRMesh,  steer);
        applySteer(wheelFrontLMesh,  steer);

        applyWheelRoll(wheelFrontRMesh, +1.0f);
        applyWheelRoll(wheelFrontLMesh, +1.0f);
        applyWheelRoll(wheelBackRMesh,  +1.0f);
        applyWheelRoll(wheelBackLMesh,  +1.0f);

        float camDistance = cameraDistance_ * scale_ + 3 * scale_;
        float camHeight = cameraHeight_ * scale_ + 2 * scale_;

        float rot = carLogic.getRotation();
        const auto forwardDir = threepp::Vector3(std::sin(rot), 0, std::cos(rot));
        const threepp::Vector3 camOffset = forwardDir * camDistance;
        threepp::Vector3 camPos = carLogic.getPosition() - camOffset;

        camPos.y += camHeight;

        constexpr float smoothingSpeed = 10.0f;
        camera->position.lerp(camPos, 1.0f - std::exp(-smoothingSpeed * dt * forwardDir.length()));
        camera->lookAt(carLogic.getPosition());
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