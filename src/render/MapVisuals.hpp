//
// Created by Mella on 29.10.2025.
//

#ifndef CARSIM_MAPVISUALS_HPP
#define CARSIM_MAPVISUALS_HPP

#include <threepp/threepp.hpp>
#include <MapLogic.hpp>
#include <CheckpointLogic.hpp>
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
    MapVisuals(std::string  mapName,const MapLogic& mapLogic, const CheckpointLogic& checkpointLogic): mapName_(std::move(mapName)), totalWidth(0), totalHeight(0) {

        mapMesh_ = std::make_shared<threepp::Group>();

        threepp::TextureLoader tl;
        const auto grassTexture = tl.load(std::string(DATA_DIR) + "/textures/grass.png");
        if (!grassTexture) {
            std::cerr << "Failed to load grass texture\n";
            return;
        }
        const auto mapSectionMat  = threepp::MeshStandardMaterial::create();
        mapSectionMat->map = grassTexture;
        mapSectionMat->color = threepp::Color::darkolivegreen;
        mapSectionMat->side = threepp::Side::Double;
        mapSectionMat->flatShading = true;
        const auto width = static_cast<float>(mapLogic.getWidth());
        const auto height = static_cast<float>(mapLogic.getWidth());

        const auto mapSectionGeo = threepp::PlaneGeometry::create(width, height);
        const auto mapSectionMesh = threepp::Mesh::create(mapSectionGeo, mapSectionMat);
        mapSectionMesh->position.set(width/2 , height/2, -0.2 );
        mapMesh_->add(mapSectionMesh);
        const std::vector<RoadSample>& roadSamples = mapLogic.getRoadSamples();
        const bool looped = mapLogic.isLooped();
        const float roadWidth = mapLogic.road.getWidth();

        createRoadMesh(roadSamples, roadWidth, looped);
        createSandShoulder(roadSamples, roadWidth, roadWidth, looped);

        showCheckPoints(checkpointLogic.getCheckpoints(), checkpointLogic.getStartCheckpoint());

    }
    std::shared_ptr<threepp::Group> getMapMesh() {
        return mapMesh_;
    }

private:
    std::shared_ptr<threepp::Group> mapMesh_;
    std::string mapName_;
    float totalWidth = 10000, totalHeight = 10000;

    void showCheckPoints(const std::vector<Checkpoint>& checkpoints, const Checkpoint& startCP) const {
        const auto finishGeometry = threepp::CylinderGeometry::create(15, 15, 300);
        const auto finishMaterial = threepp::MeshBasicMaterial::create();
        finishMaterial->color = threepp::Color::rebeccapurple;
        const threepp::Vector3 finishFwd = startCP.forward;
        const threepp::Vector3 finishLateral = threepp::Vector3(-finishFwd.y, finishFwd.x, 0).normalize(); // Z up
        const Vector3 finishLeftEdge  = startCP.position - finishLateral * (startCP.width * 0.5f);
        const Vector3 finishRightEdge = startCP.position + finishLateral * (startCP.width * 0.5f);

        const auto finishMeshLeft = threepp::Mesh::create(finishGeometry, finishMaterial);
        finishMeshLeft->position = finishLeftEdge;
        finishMeshLeft->rotation.x = threepp::math::degToRad(90);
        mapMesh_->add(finishMeshLeft);

        const auto finishMeshRight = threepp::Mesh::create(finishGeometry, finishMaterial);
        finishMeshRight->position = finishRightEdge;
        finishMeshRight->rotation.x = threepp::math::degToRad(90);
        mapMesh_->add(finishMeshRight);

        for (auto& check : checkpoints) {
            const auto geometry = threepp::CylinderGeometry::create(10, 10, 100);
            const auto material = threepp::MeshBasicMaterial::create();
            material->color = threepp::Color::saddlebrown;

            const threepp::Vector3 fwd = check.forward;
            const threepp::Vector3 lateral = threepp::Vector3(-fwd.y, fwd.x, 0).normalize(); // Z up

            const Vector3 leftEdge  = check.position - lateral * (check.width * 0.5f);
            const Vector3 rightEdge = check.position + lateral * (check.width * 0.5f);

            // Left marker
            auto meshLeft = threepp::Mesh::create(geometry, material);
            meshLeft->position = leftEdge;
            meshLeft->rotation.x = threepp::math::degToRad(90);

            mapMesh_->add(meshLeft);

            // Right marker
            auto meshRight = threepp::Mesh::create(geometry, material);
            meshRight->position = rightEdge;
            meshRight->rotation.x = threepp::math::degToRad(90);
            mapMesh_->add(meshRight);
        }
    }

    void createRoadMesh(const std::vector<RoadSample>& roadSamples,const float roadWidth = 250.f, const bool looped = true) const {
        if (roadSamples.empty()) return;

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
        const auto geometry = threepp::BufferGeometry::create();
        geometry->setAttribute("position", threepp::FloatBufferAttribute::create(vertices, 3));
        geometry->setAttribute("uv", threepp::FloatBufferAttribute::create(uvs, 2));
        geometry->setIndex(indices);
        geometry->computeVertexNormals();

        threepp::TextureLoader tl;
        const std::string texturePath = std::string(DATA_DIR) + "/textures/road.png";
        const auto texture = tl.load(texturePath);
        const auto material = threepp::MeshStandardMaterial::create({
            {"color", threepp::Color::darkgray},
            {"side", threepp::Side::Double}
        });

        texture->wrapS = texture->wrapT = threepp::TextureWrapping::Repeat;
        texture->repeat.set(2, 2);
        material->map = texture;
        material->roughness = 1.0f;
        material->metalness = 0.1f;
        material->needsUpdate();

        mapMesh_->add(threepp::Mesh::create(geometry, material));
    }

    void createSandShoulder(const std::vector<RoadSample>& samples,const float roadWidth = 250.f, const float shoulderWidth = 400.f, bool looped = true) const {
        if (samples.empty()) return;

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

            vertices.insert(vertices.end(), {innerLeft.x, innerLeft.y, innerLeft.z});
            vertices.insert(vertices.end(), {outerLeft.x, outerLeft.y, outerLeft.z});
            vertices.insert(vertices.end(), {innerRight.x, innerRight.y, innerRight.z});
            vertices.insert(vertices.end(), {outerRight.x, outerRight.y, outerRight.z});

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

        const auto geometry = threepp::BufferGeometry::create();
        geometry->setAttribute("position", threepp::FloatBufferAttribute::create(vertices, 3));
        geometry->setAttribute("uv", threepp::FloatBufferAttribute::create(uvs, 2));
        geometry->setIndex(indices);
        geometry->computeVertexNormals();

        threepp::TextureLoader tl;
        const auto texture = tl.load(std::string(DATA_DIR) + "/textures/sand.png");
        if (!texture) {
            std::cerr << "Failed to load sand texture\n";
            return;
        }
        texture->wrapS = texture->wrapT = threepp::TextureWrapping::Repeat;
        texture->repeat.set(1, 1);

        const auto material = threepp::MeshStandardMaterial::create();
        material->side = threepp::Side::Double;
        material->map = texture;
        material->color = threepp::Color::burlywood;
        material->roughness = 1.0f;
        material->metalness = 0.0f;
        material->needsUpdate();

        mapMesh_->add(threepp::Mesh::create(geometry, material));
    }

};

#endif //CARSIM_MAPVISUALS_HPP