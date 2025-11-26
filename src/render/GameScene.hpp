

#ifndef CARSIM_GAMESCENE_HPP
#define CARSIM_GAMESCENE_HPP

#include <map>
#include <vector>
#include <threepp/threepp.hpp>

#include "CarVisuals.hpp"
#include "ResourceManager.hpp"
#include "GameLogic.hpp"
#include "IScene.hpp"

#
class GameScene : public IScene {
public:
    std::shared_ptr<threepp::Scene> scene_;
    GameScene(
        const GameState& state,
        ResourceManager& resources,
        const std::string& playerModel
    ) :
        scene_(threepp::Scene::create())
    {
        setupLighting();
        setupFog();
        setupCamera(state.cars.front().position);
        buildRoadMesh(resources, state.mapData.roadSamples, state.mapData.roadWidth);
        buildSandShoulder(resources, state.mapData.roadSamples, state.mapData.roadWidth);
        buildGroundVisuals(resources, state.mapData.width, state.mapData.height);
        buildCheckPointsVisuals(state.checkpointsData.checkpoints, state.checkpointsData.startpoint);
        buildCarVisuals(state.cars, playerModel, state.scale, resources );
        buildPickupVisuals(resources, state.pickups, state.scale);
        //printSceneHierarchy();

    }

    static void printHierarchy(const threepp::Object3D& obj, const int depth = 0) {
        const std::string indent(depth * 2, ' ');
        for (const auto& child : obj.children) {
            if (child) printHierarchy(*child, depth + 1);
        }
    }
    void printSceneHierarchy() const {
        std::cout << "\n=== Scene Hierarchy ===\n";
        std::cout << "Root Scene has " << scene_->children.size() << " children\n\n";
        for (const auto& child : scene_->children) {
            if (child) printHierarchy(*child);
        }
    }

    void update(const GameState& state, const float dt) const {
        const size_t count = std::min(state.cars.size(), carVisuals_.size());
        for (size_t i = 0; i < count; i++) {
            const auto& car = state.cars[i];
            carVisuals_[i]->update(car.position, car.rotation, car.speed, car.steering, dt);
            if (i == 0) {
                updateCameraFollow(car, state.scale, dt);
            }
        }
        for (size_t i = 0; i < state.pickups.size() && i < pickupMeshes_.size(); ++i) {
            const auto& pickup = state.pickups[i];
            const auto& pickupMesh = pickupMeshes_[i];
            pickupMesh->visible = pickup.active;
            if (pickup.active) {
                pickupMesh->rotation.z += dt * 8.f;
                pickupMesh->position.set(pickup.position.x,
                                         pickup.position.y, 0);
            }
        }
    }
    void render(threepp::GLRenderer& renderer) override {
        if (scene_ && camera_) {
            renderer.render(*scene_, *camera_);
        }
    }

private:
    float scale_ = 1.0f;
    float cameraUpdateInput = 0.f;
    float cameraDistance_   = 4.f;
    float cameraHeight_     = 4.f;

    std::shared_ptr<threepp::PerspectiveCamera> camera_;

    std::shared_ptr<threepp::Mesh> roadMesh_;
    std::shared_ptr<threepp::Mesh> sandMesh_;
    std::shared_ptr<threepp::Mesh> groundMesh_;
    std::shared_ptr<threepp::Group> pickupGroup_;
    std::shared_ptr<threepp::Group> checkpointGroup_;
    std::vector<std::shared_ptr<threepp::Group>> pickupMeshes_;
    std::vector<std::shared_ptr<CarVisuals>> carVisuals_;

    void setupLighting() const {
        const auto ambient = threepp::AmbientLight::create(threepp::Color::white, 0.8f);
        const auto light = threepp::DirectionalLight::create(threepp::Color::white, 0.5f);
        light->position.set(100, 100, 200);
        scene_->add(ambient);
        scene_->add(light);
    }

    void setupFog() const {
        scene_->fog = threepp::Fog(threepp::Color::whitesmoke, 200, 8000);
    }

    void setupCamera(const threepp::Vector3& startPos) {
        camera_ = threepp::PerspectiveCamera::create(80, 16.f/9.f, 0.1f, 10000.f);
        camera_->position = startPos;
        camera_->lookAt(startPos);
        camera_->up = threepp::Vector3(0, 0, 1);
    }

    void buildCarVisuals(const std::vector<CarData>& cars, const std::string& playerModel, float scale, ResourceManager& resources) {
        carVisuals_.reserve(cars.size());
        const std::vector<std::string> colors = {"blue","green","yellow","pink","white","purple","orange", "gray", "darkgreen", "golden"};

        for (size_t i = 0; i < cars.size(); i++) {
            std::string model = (i == 0) ? playerModel : "redcar";
            std::string color = (i == 0) ? "basic" : colors[i % colors.size()];
            auto vis = std::make_shared<CarVisuals>(
                model,
                cars[i].position,
                cars[i].rotation,
                cars[i].carSize,
                scale,
                resources,
                color
            );
            carVisuals_.push_back(vis);;
            scene_->add(vis->getCarMesh());
        }
    }

    void buildGroundVisuals(ResourceManager& resources, const int width, int height) {
        const auto grassTexture = resources.loadTexture(std::string(DATA_DIR) + "/textures/grass.png");
        const auto groundMat = threepp::MeshBasicMaterial::create();
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

        groundMesh_ = threepp::Mesh::create(groundGeo, groundMat);
        groundMesh_->name = "groundMesh";
        groundMesh_->position.set(width / 2, height / 2 ,-20 );
        scene_->add(groundMesh_);
    }

    void buildSandShoulder(ResourceManager& resources, const std::vector<RoadSample>& samples,const float roadWidth = 250.f, const float shoulderWidth = 400.f, bool looped = true) {

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


        const auto sandTexture = resources.loadTexture(std::string(DATA_DIR) + "/textures/sand.png");
        const auto sandMaterial = threepp::MeshStandardMaterial::create();

        sandGeometry->getAttribute("position")->needsUpdate();
        sandGeometry->getAttribute("uv")->needsUpdate();
        sandGeometry->computeVertexNormals();
        sandGeometry->computeBoundingBox();

        sandMaterial->side = threepp::Side::Double;
        if (sandTexture) {
            sandTexture->wrapS = threepp::TextureWrapping::Repeat;
            sandTexture->wrapT = threepp::TextureWrapping::Repeat;
            sandTexture->repeat.set(1, 1);
            sandMaterial->map = sandTexture;
            sandMaterial->color = threepp::Color::white;
            sandMaterial->needsUpdate();
        } else {
            sandMaterial->color = threepp::Color::burlywood;
        }
        sandMesh_ = threepp::Mesh::create(sandGeometry, sandMaterial);
        roadMesh_->name = "sandMesh";
        scene_->add( sandMesh_);
    }

    void buildRoadMesh(ResourceManager& resources, const std::vector<RoadSample>& roadSamples,const float roadWidth = 250.f, const bool looped = true) {
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

        const auto roadTexture = resources.loadTexture(std::string(DATA_DIR) + "/textures/road.png");
        const auto roadMaterial = threepp::MeshStandardMaterial::create();

        roadGeometry->getAttribute("position")->needsUpdate();
        roadGeometry->getAttribute("uv")->needsUpdate();
        roadGeometry->computeVertexNormals();
        roadGeometry->computeBoundingBox();

        if(roadTexture) {
            roadTexture->wrapS = roadTexture->wrapT = threepp::TextureWrapping::Repeat;
            roadTexture->repeat.set(2, 2);
            roadMaterial->map = roadTexture;
            roadMaterial->color = threepp::Color::white;
            roadMaterial->needsUpdate();
        } else {
            roadMaterial->color = threepp::Color::gray;
        }
        roadMaterial->side = threepp::Side::Double;
        roadMesh_ = threepp::Mesh::create(roadGeometry, roadMaterial);
        roadMesh_->name = "roadMesh";
        scene_->add(roadMesh_);
    }

    void buildCheckPointsVisuals(const std::vector<Checkpoint>& checkpoints, const Checkpoint& startCP) {
        checkpointGroup_ = threepp::Group::create();
        checkpointGroup_->name = "CheckpointGroup";

        const auto finishGeometry = threepp::CylinderGeometry::create(15, 15, 300);
        const auto finishMaterial = threepp::MeshBasicMaterial::create();
        finishMaterial->color = threepp::Color::rebeccapurple;
        finishMaterial->needsUpdate();
        const threepp::Vector3 finishFwd = startCP.forward;
        const threepp::Vector3 finishLateral = threepp::Vector3(-finishFwd.y, finishFwd.x, 0).normalize(); // Z up
        const Vector3 finishLeftEdge  = startCP.position - finishLateral * (startCP.width * 0.5f);
        const Vector3 finishRightEdge = startCP.position + finishLateral * (startCP.width * 0.5f);

        const auto finishMeshLeft = threepp::Mesh::create(finishGeometry, finishMaterial);
        finishMeshLeft->position = finishLeftEdge;
        finishMeshLeft->rotation.x = threepp::math::degToRad(90);
        checkpointGroup_->add(finishMeshLeft);

        const auto finishMeshRight = threepp::Mesh::create(finishGeometry, finishMaterial);
        finishMeshRight->position = finishRightEdge;
        finishMeshRight->rotation.x = threepp::math::degToRad(90);
        checkpointGroup_->add(finishMeshRight);

        for (auto& check : checkpoints) {
            const auto geometry = threepp::CylinderGeometry::create(10, 10, 100);
            const auto material = threepp::MeshBasicMaterial::create();
            material->color = threepp::Color::saddlebrown;

            const threepp::Vector3 fwd = check.forward;
            const threepp::Vector3 lateral = threepp::Vector3(-fwd.y, fwd.x, 0).normalize(); // Z up

            const Vector3 leftEdge  = check.position - lateral * (check.width * 0.5f);
            const Vector3 rightEdge = check.position + lateral * (check.width * 0.5f);

            auto meshLeft = threepp::Mesh::create(geometry, material);
            meshLeft->position = leftEdge;
            meshLeft->rotation.x = threepp::math::degToRad(90);

            checkpointGroup_->add(meshLeft);

            auto meshRight = threepp::Mesh::create(geometry, material);
            meshRight->position = rightEdge;
            meshRight->rotation.x = threepp::math::degToRad(90);

            checkpointGroup_->add(meshRight);
        }
        scene_->add(checkpointGroup_);
    }

    void buildPickupVisuals(ResourceManager& resources, const std::vector<PickupData> &pickups, const float scale) {
        pickupGroup_ = threepp::Group::create();
        pickupGroup_->name = "PickupGroup";
        for (size_t i = 0; i < pickups.size(); ++i) {
            const auto& pickup = pickups[i];
            std::shared_ptr<threepp::Group> mesh;
            switch (pickup.type) {
                case PickupType::Coin: {
                    std::string Dir = std::string(DATA_DIR) + "/objects/Pickups/coin.obj";
                    mesh = resources.cloneInstance(Dir);
                    break;
                }
                case PickupType::BadCoin: {
                    std::string Dir = std::string(DATA_DIR) + "/objects/Pickups/coin.obj";
                    mesh = resources.cloneInstance(Dir);
                    break;
                }
                default: ;
            }
            if (mesh) {
                mesh->name = "Pickup_" + std::to_string(i) + "_" + std::to_string(static_cast<int>(pickup.type));
                mesh->position.set(pickup.position.x, pickup.position.y, 0);
                mesh->scale = Vector3(pickup.radius * 2.f,pickup.radius * 2.f, pickup.radius * 2.f);
                pickupGroup_->add(mesh);
                pickupMeshes_.emplace_back(mesh);
            }
        }
        scene_->add(pickupGroup_);
    }

    void updateCameraFollow(const CarData& car, const float scale, const float dt) const {
        const auto pos = car.position;
        const threepp::Vector3 forward = car.forward;

        const float dist  = (cameraDistance_+ (0.5f * car.boostFactor) ) * scale;
        const float height = cameraHeight_ * scale;

        threepp::Vector3 desired = pos - forward * dist;
        desired.z += height;

        const float smooth = 1.f - std::pow(1.f - 0.2f, dt * 60.f);
        camera_->position.lerp(desired, smooth);

        camera_->lookAt(pos);
    }
};

#endif // CARSIM_GAMESCENE_HPP
