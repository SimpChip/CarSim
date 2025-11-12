//
// Created by Mella on 29.10.2025.
//

#ifndef CARSIM_MAPVISUALS_HPP
#define CARSIM_MAPVISUALS_HPP

#include <threepp/threepp.hpp>
#include <MapLogic.hpp>
#include <random>



class MapVisuals {
public:
    MapLogic& mapLogic_;

    MapVisuals(std::string mapName, MapLogic& mapLogic): mapLogic_(mapLogic) , mapName_(mapName), totalWidth(0), totalHeight(0) {
        threepp::TextureLoader tl;
        threepp::OBJLoader ol;
        mapMesh_ = std::make_shared<threepp::Group>();

        loadMapParts(mapName_, tl);
        createNaturalBorder(ol, tl);

    }
    std::shared_ptr<threepp::Group> getMapMesh() {
        return mapMesh_;
    }

private:
    std::shared_ptr<threepp::Group> mapMesh_;
    std::string mapName_;
    float totalWidth, totalHeight;

    void loadMapParts(const std::string& mapName, threepp::TextureLoader& tl) {

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
            auto mapSectionMesh = createMapPart(x, y, texture);
            mapMesh_->add(mapSectionMesh);
        }
        std::cout << "GameScene is created" << std::endl;
    }

    std::shared_ptr<threepp::Mesh> createMapPart(int Mx,int My,std::shared_ptr<threepp::Texture> texture)
    {
        int mapWidth = mapLogic_.width_;
        int mapHeight = mapLogic_.height_;
        totalHeight = static_cast<float>(My+1)*mapHeight;
        totalWidth = static_cast<float>(Mx+1)*mapWidth;
        auto mapSectionMat  = threepp::MeshStandardMaterial::create();
        mapSectionMat->flatShading = true;

        auto mapSectionGeo = threepp::PlaneGeometry::create(mapWidth, mapHeight, mapWidth/4, mapHeight/4);
        texture->magFilter = threepp::Filter::Linear;
        texture->minFilter = threepp::Filter::LinearMipmapLinear;
        texture->wrapS = threepp::TextureWrapping::Repeat;
        texture->wrapT = threepp::TextureWrapping::Repeat;
        texture->repeat.set(4, 4);
        mapSectionMat->map = texture;

        auto mapSectionMesh = threepp::Mesh::create(mapSectionGeo, mapSectionMat);
        mapSectionMesh->position.set(mapWidth/2 + mapWidth * Mx , mapHeight/2 + mapHeight * My, -0.1 );

        const auto position = mapSectionMesh->geometry()->getAttribute<float>("position");
        position->setUsage(threepp::DrawUsage::Dynamic);
        position->needsUpdate();

        for (size_t i = 0; i < position->count(); ++i) {
            float x = position->getX(i);
            float y = position->getY(i);

            float zOffset = 0;
            const float baseZ = -0.1f;
            int px = static_cast<int>((x + mapWidth/ 2));
            int py = static_cast<int>((y +  mapHeight/ 2));

            if (px >= 0 && py >= 0 && px < mapLogic_.width_ && py < mapLogic_.height_) {
                if (mapLogic_.getBit(mapLogic_.roadMask_, px/mapLogic_.scale_, py/mapLogic_.scale_)) {
                    zOffset += 0.6f + rand()%2*0.1f;
                }
            }

            position->setZ(i, baseZ + zOffset);
        }

        return mapSectionMesh;
    }

    void createNaturalBorder(threepp::OBJLoader& ol , threepp::TextureLoader& tl) {
        const auto geometry = threepp::PlaneGeometry::create(totalWidth *2, totalHeight*2, totalWidth/10, totalHeight/10);
        const auto material = threepp::MeshStandardMaterial::create();

        std::string texturePath = std::string(DATA_DIR) + "/textures/grass.png";
        auto texture = tl.load(texturePath);
        texture->magFilter = threepp::Filter::Linear;
        texture->minFilter = threepp::Filter::LinearMipmapLinear;
        texture->wrapS = threepp::TextureWrapping::Repeat;
        texture->wrapT = threepp::TextureWrapping::Repeat;
        texture->repeat.set(4, 4);
        material->map = texture;
        material->flatShading = true;
        auto mesh = threepp::Mesh::create(geometry, material);
        const auto wireframe = threepp::Mesh::create(geometry, threepp::MeshBasicMaterial::create(
                                                              {{"wireframe", true},
                                                               {"color", threepp::Color::darkgray}}));
        //mesh->add(wireframe);
        mesh->position.set(totalHeight/2, totalWidth/2, 0);

        const auto position = mesh->geometry()->getAttribute<float>("position");
        position->setUsage(threepp::DrawUsage::Dynamic);

        for (size_t i = 0; i < position->count(); ++i) {
            float x = position->getX(i);
            float y = position->getY(i);

            float mapRadius = 0.5f * std::sqrt(totalWidth * totalWidth + totalHeight * totalHeight);
            float distance = sqrt(x * x + y * y);
            float edgeFactor = 0.0f;

            if (distance > mapRadius) {
                float riseZone = 100.f; // thickness of the slope
                edgeFactor = std::clamp((distance - mapRadius) / riseZone, 0.5f, 10.f);
            }

            float jitter = ((rand() % 100) / 100.f - 0.5f) * 10.f;
            float zOffset = edgeFactor * (30 + jitter);
            const float baseZ = 0.f;

            int px = static_cast<int>((x + totalWidth / 2) / mapLogic_.scale_);
            int py = static_cast<int>((y + totalHeight / 2) / mapLogic_.scale_);

            if (px >= 0 && py >= 0 && px < mapLogic_.scaledwidth_ && py < mapLogic_.scaledheight_) {
                if (mapLogic_.getBit(mapLogic_.obstacleMask_, px, py)) {
                    float localJitter = ((rand() % 100) / 100.f - 0.5f) * 10.f;
                    zOffset += 20.f + localJitter;
                }
            }

            position->setZ(i, baseZ + zOffset);
        }
        position->needsUpdate();

        const std::string dir = std::string(DATA_DIR) + "/objects";
        const int treeCount = 1000;

        struct ObjType {
            std::string path;
            float scale;
        };

        std::vector types = {
            ObjType("Trees/Tree1.1.obj", 30),
            ObjType("Stones/Stone1.1.obj", 20),
            ObjType("Misc/Mushroom1.1.obj", 100),
            ObjType("Misc/Flower1.1.obj", 150),
            ObjType("Trees/Tree3.2.obj", 20),
        };

        std::mt19937 rng(1337);
        std::uniform_int_distribution<int> pick(0, position->count() - 1);

        float mapRadius = 0.5f * std::sqrt(totalWidth * totalWidth + totalHeight * totalHeight);
        float innerBorder = mapRadius * 1.05f;
        float outerBorder = mapRadius * 1.20f;

        int placed = 0;
        int maxAttempts = treeCount * 5;

        for (int attempts = 0; attempts < maxAttempts && placed < treeCount; ++attempts) {
            int random_type = rand() % static_cast<int>(types.size());
            int idx = pick(rng);

            float vx = position->getX(idx);
            float vy = position->getY(idx);
            float vz = position->getZ(idx);


            int px = static_cast<int>((vx + totalWidth / 2) / mapLogic_.scale_);
            int py = static_cast<int>((vy + totalHeight / 2) / mapLogic_.scale_);
            float distance = std::sqrt(vx * vx + vy * vy);

            if (distance < innerBorder || distance > outerBorder) {
                if (px >= 0 && py >= 0 && px < mapLogic_.scaledwidth_ && py < mapLogic_.scaledheight_) {
                    if (mapLogic_.getBit(mapLogic_.obstacleMask_, px, py)) {} else {
                        continue;
                    }
                } else {
                    continue;
                }
            }

            std::string newDir = dir + "/" + types[random_type].path;
            auto instancedMesh = ol.load(newDir, true);

            threepp::Vector3 pos(vx, vy, vz);
            threepp::Quaternion rot;
            float random_size = static_cast<float>((rand() % 20) - 10);
            float scaleFactor = types[random_type].scale + random_size;
            instancedMesh->position = pos;
            instancedMesh->scale = threepp::Vector3(scaleFactor, scaleFactor, scaleFactor);

            mesh->add(instancedMesh);
            placed++;
        }

        std::cout << "Placed " << placed << " natural objects outside the map border.\n";
        mapMesh_->add(mesh);

    }
};

#endif //CARSIM_MAPVISUALS_HPP