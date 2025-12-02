
#include "scenes/GameScene.hpp"
#include <random>

GameScene::GameScene(
    const GameSnap& state,
    ResourceManager& rm,
    const std::string& playerModel
) :
    scene_(threepp::Scene::create()) {
    setupLighting();
    setupFog();
    setupCamera(state.cars.front().position);
    buildRoadMesh(rm, state.map.roadSamples, state.map.roadWidth, state.map.looped);
    buildSandShoulder(rm, state.map.roadSamples, state.map.roadWidth, 400, state.map.looped);
    buildGroundVisuals(rm, state.map.width, state.map.height);
    buildCheckPointsVisuals(state.lap.checkpoints);
    buildCarVisuals(state.cars, playerModel, state.scale, rm );
    buildPickupVisuals(rm, state.pickups, state.scale);
}

void GameScene::update(const GameSnap& state, const float dt) const {

    for (size_t i = 0; i < state.cars.size() && i < carVisuals_.size() ; i++) {
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
            pickupMesh->rotation.z += pickup.rotation;
            pickupMesh->position.set(pickup.position.x,
                                     pickup.position.y, 0);
        }
    }

    const auto& lap = state.lap;
    if (!lap.laptrackers.empty()) {
        const auto& tracker = lap.laptrackers.at(state.playerCarId);
        const int currentCp = tracker.currentCheckpoint;
        const int nextCp    = tracker.nextCheckpoint;
        const int startCp   = lap.startIndex;
        const bool raceStarted = lap.startTimer <= 0.f && lap.raceTimer > 0.f;

        for (size_t childIdx = 0; childIdx < checkpointGroup_->children.size(); ++childIdx) {
            constexpr int stride = 2;
            const int cpIdx = static_cast<int>(childIdx / stride);
            const auto& child = checkpointGroup_->children[childIdx];
            const auto mat = std::dynamic_pointer_cast<threepp::MeshBasicMaterial>(child->material());
            if (!mat) {
                std::cerr << "Checkpoint child material missing at index " << childIdx << std::endl;
                continue;
            }
            if (cpIdx == startCp) {
                if (lap.startTimer > 0.f) {
                    // Pre-race countdown
                    if (lap.startTimer > 2.f) {
                        mat->color = threepp::Color::red;
                    } else if (lap.startTimer > 1.f) {
                        mat->color = threepp::Color::yellow;
                    } else {
                        mat->color = threepp::Color::limegreen;
                    }
                } else if (cpIdx == nextCp && raceStarted) {
                    mat->color = threepp::Color::magenta;
                } else if (cpIdx == currentCp && raceStarted) {
                    mat->color = threepp::Color::white;
                } else {
                    mat->color = threepp::Color::blue;
                }
            }
            else {
                if (cpIdx == currentCp) {
                    mat->color = threepp::Color::cyan;
                } else if (cpIdx == nextCp) {
                    mat->color = threepp::Color::yellowgreen;
                } else {
                    mat->color = threepp::Color::firebrick;
                }
            }
        }
    }
}

void GameScene::render(threepp::GLRenderer& renderer) const {
    if (scene_ && camera_) {
        renderer.render(*scene_, *camera_);
    }
}

void GameScene::setupLighting() const {
    const auto ambient = threepp::AmbientLight::create(threepp::Color::white, 0.8f);
    const auto light = threepp::DirectionalLight::create(threepp::Color::white, 0.5f);
    light->position.set(100, 100, 200);
    scene_->add(ambient);
    scene_->add(light);
}

void GameScene::setupFog() const {
    scene_->fog = threepp::Fog(threepp::Color::whitesmoke, 200, 8000);
}

void GameScene::setupCamera(const threepp::Vector3& startPos) {
    camera_ = threepp::PerspectiveCamera::create(80, 16.f/9.f, 0.1f, 10000.f);
    camera_->position = startPos;
    camera_->lookAt(startPos);
    camera_->up = threepp::Vector3(0, 0, 1);
}

void GameScene::buildCarVisuals(const std::vector<CarSnap>& cars, const std::string& playerModel, float scale, ResourceManager& resources) {
    carVisuals_.reserve(cars.size());
    const std::vector<std::string> colors = {"blue","green","yellow","pink","white","purple","orange", "gray", "darkgreen", "golden"};

    for (size_t i = 0; i < cars.size(); i++) {
        std::string model = (i == 0) ? playerModel : "redcar";
        std::string color = (i == 0) ? "basic" : colors[i % colors.size()];
        auto vis = std::make_shared<CarVisuals>(
            "redcar",
            "basic",
            cars[i].position,
            cars[i].rotation,
            cars[i].size,
            scale,
            resources
        );
        carVisuals_.push_back(vis);;
        scene_->add(vis->getCarMesh());
    }
}


void GameScene::buildGroundVisuals(ResourceManager& rm, const int width, const int height) {
    const auto grassTexture = rm.loadTexture(std::string(DATA_DIR) + "/textures/grass.png");
    const auto groundMat = threepp::MeshBasicMaterial::create();
    if (!grassTexture) {
        std::cerr << "Failed to load grass texture\n";
        groundMat->color = threepp::Color::green;
    } else {
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


void GameScene::buildSandShoulder(
    ResourceManager& rm,
    const std::vector<RoadSample>& samples,
    const float roadWidth = 250.f,
    const float shoulderWidth = 400.f,
    const bool looped = true)
{
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
        threepp::Vector3 innerLeft  = s.pos + s.right * halfRoad;
        threepp::Vector3 innerRight = s.pos - s.right * halfRoad;
        threepp::Vector3 outerLeft  = s.pos + s.right * halfShoulder;
        threepp::Vector3 outerRight = s.pos - s.right * halfShoulder;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(-0.5f, -0.1f);

        const float zOffset1 = dist(gen);
        const float zOffset2 = dist(gen);
        const float zOffset3 = dist(gen);
        const float zOffset4 = dist(gen);
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


    const auto sandTexture = rm.loadTexture(std::string(DATA_DIR) + "/textures/sand.png");
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


void GameScene::buildRoadMesh(ResourceManager& resources, const std::vector<RoadSample>& samples,const float roadWidth = 250.f, const bool looped = true) {
    const float halfWidth = roadWidth * 0.5f;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> uvs;
    float accumulatedLength = 0.0f;

    for (size_t i = 0; i < samples.size(); ++i) {
        const auto& s = samples[i];
        threepp::Vector3 left  = s.pos + s.right * halfWidth;
        threepp::Vector3 right = s.pos - s.right * halfWidth;

        vertices.push_back(left.x); vertices.push_back(left.y); vertices.push_back(left.z);
        vertices.push_back(right.x); vertices.push_back(right.y); vertices.push_back(right.z);

        if (i > 0) {
            accumulatedLength += (samples[i].pos - samples[i-1].pos).length();
        }
        float u = accumulatedLength / 500.0f;
        uvs.push_back(u); uvs.push_back(0);
        uvs.push_back(u); uvs.push_back(1);
    }

    for (size_t i = 0; i < samples.size()-1; ++i) {
        unsigned int i0 = i * 2;
        unsigned int i1 = i0 + 1;
        unsigned int i2 = i0 + 2;
        unsigned int i3 = i0 + 3;

        indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
        indices.push_back(i2); indices.push_back(i3); indices.push_back(i1);
    }

    if (looped && samples.size() > 2) {
        const unsigned int i0 = (samples.size()-1) * 2;
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


void GameScene::buildCheckPointsVisuals(const std::vector<CpSnap>& checkpoints) {
    checkpointGroup_ = threepp::Group::create();
    checkpointGroup_->name = "CheckpointGroup";

    for (const auto&[position, forward, width, start] : checkpoints) {

        threepp::Vector3 fwd = forward;
        fwd.normalize();
        threepp::Vector3 lateral(-fwd.y, fwd.x, 0); ;

        const auto geometry = threepp::CylinderGeometry::create(10, 10, 100);
        const auto material = threepp::MeshBasicMaterial::create();

        material->color = start ? threepp::Color::indianred : threepp::Color::green;
        material->wireframe = true;

        const threepp::Vector3 leftEdge  = position - lateral * (width * 0.5f);
        const threepp::Vector3 rightEdge = position + lateral * (width * 0.5f);

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


void GameScene::buildPickupVisuals(ResourceManager& rm, const std::vector<PickupSnap> &pickups, const float scale) {
    pickupGroup_ = threepp::Group::create();
    pickupGroup_->name = "PickupGroup";
    for (size_t i = 0; i < pickups.size(); ++i) {
        const auto& pickup = pickups[i];
        std::shared_ptr<threepp::Group> mesh;
        switch (pickup.type) {
            case PickupType::Coin: {
                std::string Dir = std::string(DATA_DIR) + "/objects/Pickups/coin.obj";
                mesh = rm.cloneInstance(Dir);
                break;
            }
            case PickupType::BadCoin: {
                std::string Dir = std::string(DATA_DIR) + "/objects/Pickups/coin.obj";
                mesh = rm.cloneInstance(Dir);
                break;
            }
            default: ;
        }
        if (mesh) {
            mesh->name = "Pickup_" + std::to_string(i) + "_" + std::to_string(static_cast<int>(pickup.type));
            mesh->position.set(pickup.position.x, pickup.position.y, 0);
            mesh->scale = threepp::Vector3(pickup.radius,pickup.radius, pickup.radius);
            pickupGroup_->add(mesh);
            pickupMeshes_.emplace_back(mesh);
        }
    }
    scene_->add(pickupGroup_);
}

void GameScene::updateCameraFollow(const CarSnap& car, const float scale, const float dt) const {
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