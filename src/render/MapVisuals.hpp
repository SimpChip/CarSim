//
// Created by Mella on 29.10.2025.
//

#ifndef CARSIM_MAPVISUALS_HPP
#define CARSIM_MAPVISUALS_HPP

#include <threepp/threepp.hpp>
#include <MapLogic.hpp>
#include <CheckpointLogic.hpp>
#include "ResourceManager.hpp"
#include <utility>

struct RoadCurve final : threepp::Curve3 {

    explicit RoadCurve(const std::vector<Vector3>& points, float scale = 1.0f)
        : points_(points), scale_(scale) {}

    void getPoint(const float t, Vector3& target) const override {
        if (points_.empty()) {
            target.set(0,0,0);
            return;
        }
        const float scaledT = t * (static_cast<float>(points_.size()) - 1);
        const int idx = static_cast<int>(std::floor(scaledT));
        const int nextIdx = std::min(idx + 1, static_cast<int>(points_.size() - 1));

        const float localT = scaledT - static_cast<float>(idx);

        const Vector3 p1 = points_[idx];
        const Vector3 p2 = points_[nextIdx];
        target.copy(p1).lerp(p2, localT).multiplyScalar(scale_);
    }
private:
    std::vector<Vector3> points_;
    float scale_;
};

class MapVisuals {
public:
    MapVisuals(std::string  mapName,const MapData& mapData, ResourceManager& resources, const std::shared_ptr<threepp::Scene>& scene): mapName_(std::move(mapName)) {
        const std::vector<RoadSample>& roadSamples = mapData.roadSamples;
        const bool looped = mapData.looped;
        const float roadWidth = mapData.roadWidth;
        scene->add(createGroundMesh(resources, mapData.height, mapData.height));
        scene->add(createRoadMesh(resources, roadSamples, roadWidth, looped));
        scene->add(createSandShoulder(resources, roadSamples, roadWidth, roadWidth, looped));
    }

private:
    std::string mapName_;

    static std::shared_ptr<threepp::Mesh> createGroundMesh(ResourceManager &resources, int width, int height) {
        auto grassTexture = resources.loadTexture(std::string(DATA_DIR) + "/textures/grass.png");
        auto groundMat = threepp::MeshBasicMaterial::create();
        if (!grassTexture) {
            std::cerr << "Failed to load grass texture\n";
            groundMat->color = threepp::Color::green;
        } else {
            std::cout << grassTexture.get()->name << "\n";
            grassTexture->wrapS = grassTexture->wrapT = threepp::TextureWrapping::Repeat;
            grassTexture->repeat.set(50, 50);
            groundMat->color = threepp::Color::white;
            groundMat->map = grassTexture;
        }
        groundMat->side = threepp::Side::Double;

        const auto groundGeo = threepp::PlaneGeometry::create(width, height);
        const auto groundMesh = threepp::Mesh::create(groundGeo, groundMat);

        groundMesh->position.set(width / 2, height / 2 ,-20 );

        return groundMesh;
    }

    static std::shared_ptr<threepp::Mesh> createRoadMesh(ResourceManager& resources, const std::vector<RoadSample>& roadSamples,const float roadWidth = 250.f, const bool looped = true) {
        const float halfWidth = roadWidth * 0.5f;
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::vector<float> uvs;

        float accumulatedLength = 0.0f;

        for (size_t i = 0; i < roadSamples.size(); ++i) {
            const auto& s = roadSamples[i];
            Vector3 left  = s.pos + s.right * halfWidth;
            Vector3 right = s.pos - s.right * halfWidth;

            vertices.push_back(left.x); vertices.push_back(left.y); vertices.push_back(left.z);
            vertices.push_back(right.x); vertices.push_back(right.y); vertices.push_back(right.z);

            if (i > 0) {
                accumulatedLength += (roadSamples[i].pos - roadSamples[i-1].pos).length();
            }
            float u = accumulatedLength / 500.0f;
            uvs.push_back(u); uvs.push_back(0);
            uvs.push_back(u); uvs.push_back(1);
        }

        for (size_t i = 0; i < roadSamples.size()-1; ++i) {
            unsigned int i0 = i * 2;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = i0 + 2;
            unsigned int i3 = i0 + 3;

            indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
            indices.push_back(i2); indices.push_back(i3); indices.push_back(i1);
        }

        if (looped && roadSamples.size() > 2) {
            const unsigned int i0 = (roadSamples.size()-1) * 2;
            const unsigned int i1 = i0 + 1;
            constexpr unsigned int i2 = 0;
            constexpr unsigned int i3 = 1;

            indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
            indices.push_back(i2); indices.push_back(i3); indices.push_back(i1);
        }
        const auto roadGeometry = threepp::BufferGeometry::create();
        roadGeometry->setAttribute("position", threepp::FloatBufferAttribute::create(vertices, 3));
        roadGeometry->setAttribute("uv", threepp::FloatBufferAttribute::create(uvs, 2));
        roadGeometry->setIndex(indices);
        roadGeometry->computeVertexNormals();
        roadGeometry->computeBoundingBox();
        roadGeometry->computeBoundingSphere();

        const auto roadTexture = resources.loadTexture(std::string(DATA_DIR) + "/textures/road.png");
        const auto roadMaterial = threepp::MeshStandardMaterial::create();

        if(roadTexture) {
            roadTexture->wrapS = roadTexture->wrapT = threepp::TextureWrapping::Repeat;
            roadTexture->repeat.set(2, 2);
            roadMaterial->map = roadTexture;
            roadMaterial->color = threepp::Color::white;
        } else {
            roadMaterial->color = threepp::Color::gray;
        }
        roadMaterial->side = threepp::Side::Double;
        return threepp::Mesh::create(roadGeometry, roadMaterial);
    }

    static std::shared_ptr<threepp::Mesh> createSandShoulder(ResourceManager& resources, const std::vector<RoadSample>& samples,const float roadWidth = 250.f, const float shoulderWidth = 400.f, bool looped = true) {

        const float halfRoad = roadWidth * 0.5f;
        const float halfShoulder = (roadWidth + shoulderWidth) * 0.5f;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::vector<float> uvs;

        float accumulatedLength = 0.0f;

        for (size_t i = 0; i < samples.size(); ++i) {
            if (i > 0) {
                accumulatedLength += (samples[i].pos - samples[i-1].pos).length();
            }

            const auto& s = samples[i];
            Vector3 innerLeft  = s.pos + s.right * halfRoad;
            Vector3 innerRight = s.pos - s.right * halfRoad;
            Vector3 outerLeft  = s.pos + s.right * halfShoulder;
            Vector3 outerRight = s.pos - s.right * halfShoulder;

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(-0.5f, -0.1f);

            float zOffset1 = dist(gen);
            float zOffset2 = dist(gen);
            float zOffset3 = dist(gen);
            float zOffset4 = dist(gen);
            vertices.insert(vertices.end(), {innerLeft.x, innerLeft.y, innerLeft.z + zOffset1});
            vertices.insert(vertices.end(), {outerLeft.x, outerLeft.y, outerLeft.z + zOffset2});
            vertices.insert(vertices.end(), {innerRight.x, innerRight.y, innerRight.z + zOffset3});
            vertices.insert(vertices.end(), {outerRight.x, outerRight.y, outerRight.z + zOffset4});

            float u = accumulatedLength / 200.0f;
            uvs.insert(uvs.end(), {u, 0});
            uvs.insert(uvs.end(), {u, 1});
            uvs.insert(uvs.end(), {u, 0});
            uvs.insert(uvs.end(), {u, 1});
        }
        for (size_t i = 0; i < samples.size()-1; ++i) {
            unsigned int base = i * 4;
            indices.insert(indices.end(), {base, base+4, base+1, base+4, base+5, base+1});
            indices.insert(indices.end(), {base+2, base+6, base+3, base+6, base+7, base+3});
        }

        if (looped && samples.size() > 2) {
            unsigned int base = (samples.size()-1) * 4;
            indices.insert(indices.end(), {base, 0, base+1, 0, 1, base+1});       // left quad
            indices.insert(indices.end(), {base+2, 2, base+3, 2, 3, base+3});     // right quad
        }

        const auto sandGeometry = threepp::BufferGeometry::create();
        sandGeometry->setAttribute("position", threepp::FloatBufferAttribute::create(vertices, 3));
        sandGeometry->setAttribute("uv", threepp::FloatBufferAttribute::create(uvs, 2));
        sandGeometry->setIndex(indices);

        sandGeometry->computeVertexNormals();
        sandGeometry->computeBoundingBox();
        sandGeometry->computeBoundingSphere();

        const auto sandTexture = resources.loadTexture(std::string(DATA_DIR) + "/textures/sand.png");
        const auto sandMaterial = threepp::MeshStandardMaterial::create();
        sandMaterial->side = threepp::Side::Double;
        if (sandTexture) {
            sandTexture->wrapS = threepp::TextureWrapping::Repeat;
            sandTexture->wrapT = threepp::TextureWrapping::Repeat;
            sandTexture->repeat.set(1, 1);
            sandMaterial->map = sandTexture;
            sandMaterial->color = threepp::Color::white;
        } else {
            sandMaterial->color = threepp::Color::burlywood;
        }
        return threepp::Mesh::create(sandGeometry, sandMaterial);
    }

    static void createOrganicBorder(const std::vector<Vector3>& points, BorderType biome) {
        if (points.empty()) return;
    }
};

#endif //CARSIM_MAPVISUALS_HPP