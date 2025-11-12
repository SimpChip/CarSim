//
// Created by Mella on 18.10.2025.
//
#ifndef MAPLOGIC_HPP
#define MAPLOGIC_HPP

#include <fstream>

#include "iostream"
#include "filesystem"
#include "threepp/threepp.hpp"
#include "CarLogic.hpp"
#include "SpatialGrid.hpp"
#include "stb_image.h"
#include "cmake-build-debug-mingw/_deps/threepp-src/src/external/nlohmann/nlohmann/json.hpp"

using Vector3 = threepp::Vector3;

struct ImageInfo {
    int width, height, channels;
    std::string name;
    unsigned char *data;
};

struct CheckPoint {
    int id;
    std::vector<Vector3> points;
};

enum class PickupType {
    Coin,
};

struct Pickup {
    PickupType type;
    Vector3 position;
    bool active = true;
    float radius = 10.0f;
    float respawnTimer = 0.0f;
};


class MapLogic {
public:
    std::vector<uint8_t> roadMask_; // 1 bit per pixel
    std::vector<uint8_t> obstacleMask_;
    std::map<int, std::vector<Vector3>> CheckPointMap_;
    SpatialGrid<size_t> checkPointGrid_;
    std::vector<Pickup> Pickups_;
    SpatialGrid<size_t> pickupGrid_;
    std::vector<size_t> inactivePickupIndices_;

    int width_{};
    int height_{};
    int scaledwidth_{};
    int scaledheight_{};
    int scale_ = 1;
    int checkpoint = 255;
    std::string filename_;

    explicit MapLogic(const std::string &filename) : pickupGrid_(100), checkPointGrid_(100), filename_(filename) {
        std::cout << "RaceGame starts loading all " << filename_ << "files" << std::endl;
        const std::vector<ImageInfo> sectionMaps = loadImageData();

        if (sectionMaps.empty()) {
            std::cerr << "Error: No map sections loaded. Aborting constructor." << std::endl;
            return;
        }

        int maxSectionX = 0, maxSectionY = 0;
        const int sectionPixelWidth = sectionMaps[0].width;
        const int sectionPixelHeight = sectionMaps[0].height;

        for (const auto &section: sectionMaps) {
            if (section.width != sectionPixelWidth || section.height != sectionPixelHeight) {
                std::cout << section.width << "x" << sectionMaps[0].width << std::endl;
                std::cerr << "Warning: Map sections have inconsistent sizes!" << std::endl;
            }
            auto [sx, sy] = parseMapPosition(section.name);
            if (sx > maxSectionX) maxSectionX = sx;
            if (sy > maxSectionY) maxSectionY = sy;
        }

        int sectionsX = maxSectionX + 1;
        int sectionsY = maxSectionY + 1;

        width_ = sectionPixelWidth * sectionsX;
        height_ = sectionPixelHeight * sectionsY;

        scaledwidth_ = width_ / scale_;
        scaledheight_ = height_ / scale_;

        const size_t maskSizeBytes = (static_cast<size_t>(scaledwidth_) * scaledheight_ + 7) / 8;
        roadMask_.resize(maskSizeBytes, 0);
        obstacleMask_.resize(maskSizeBytes, 0);

        for (const auto &section: sectionMaps) {
            std::cout << "Reading and Storing filedata from file: " << section.name << std::endl;
            auto [sectionX, sectionY] = parseMapPosition(section.name);

            const int sectionScaledWidth = section.width / scale_;
            const int sectionScaledHeight = section.height / scale_;

            const int offsetX = sectionX * sectionScaledWidth;
            const int offsetY = sectionY * sectionScaledHeight;

            for (int y = 0; y < sectionScaledHeight; ++y) {
                for (int x = 0; x < sectionScaledWidth; ++x) {
                    const int px = x * scale_ + scale_ / 2;
                    const int py = y * scale_ + scale_ / 2;
                    const int idx = (py * section.width + px) * 3;

                    const int r = section.data[idx];
                    const int g = section.data[idx + 1];
                    const int b = section.data[idx + 2];

                    int maskX = x + offsetX;
                    int maskY = (sectionScaledHeight - 1 - y) + offsetY;

                    if (r == 255 && g == 255) {
                        CheckPointMap_[b].emplace_back(maskX, maskY, 0);
                        setBit(roadMask_, maskX, maskY);
                    } else if (r == 255) setBit(roadMask_, maskX, maskY);
                    else if (r == 128) setBit(obstacleMask_, maskX, maskY);
                    else if (b == 255) {
                        Pickups_.emplace_back(Pickup{PickupType::Coin, Vector3(maskX, maskY, 0)});
                        setBit(roadMask_, maskX, maskY);
                    }
                }
            }
            stbi_image_free(section.data);
        }

        for (size_t i = 0; i < Pickups_.size(); ++i) {
            pickupGrid_.insert(Pickups_[i].position, i);
        }
        std::cout << "RaceGame is Created" << std::endl;
    }

    static std::pair<int, int> parseMapPosition(const std::string &filename) {
        // Expects format: "X_Y"
        size_t slash_pos = filename.find('/');
        std::string position_part = (slash_pos == std::string::npos) ? filename : filename.substr(0, slash_pos);
        size_t underscore_pos = position_part.find('_');

        if (underscore_pos == std::string::npos) {
            std::cerr << "Error: Invalid map file name format. Missing X_Y part: " << filename << std::endl;
            return {-1, -1};
        }
        int x = std::stoi(position_part.substr(0, underscore_pos));
        int y = std::stoi(position_part.substr(underscore_pos + 1));

        return {x, y};
    }

    std::vector<ImageInfo> loadImageData() const {
        std::vector<ImageInfo> images;
        for (const std::filesystem::path directoryPath{(std::string(DATA_DIR) + "/maps/" + filename_ + "/")}; const auto &entry: std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_directory()) {
                int w, h, c;
                unsigned char *data = stbi_load((entry.path().string() + "/md.png").c_str(), &w, &h, &c, 3);
                if (!data) {
                    std::cerr << "Failed to load map meta data image: " << entry.path().extension().string() <<
                            std::endl;
                    std::cerr << "stb_image error: " << stbi_failure_reason() << std::endl;
                    continue;
                }
                images.push_back({w, h, c, entry.path().filename().string(), data});
            }
        }
        return images;
    }

    void handleMapCollisions(CarLogic& car, float dt) {
        const Vector3 prevPos = car.getPreviousPosition();
        const Vector3 pos = car.getPosition();

        // Check if car is outside map bounds or radius
        float mapRadius = 0.5f * std::sqrt(width_ * width_ + height_ * height_);
        float dx = pos.x - width_ / 2.0f;
        float dy = pos.y - height_ / 2.0f;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > mapRadius) {
            car.setPosition(prevPos);
            car.applyBounce();
            return;
        }

        OBB carBox = car.getOrientedBoundingBox();
        auto corners = carBox.getCorners();

        // Check each corner of the car's bounding box against the obstacle mask
        for (const auto& corner : corners) {
            int cx = static_cast<int>(corner.x / scale_);
            int cy = static_cast<int>(corner.y / scale_);

            if (cx < 0 || cy < 0 || cx >= scaledwidth_ || cy >= scaledheight_) continue;

            if (getBit(obstacleMask_, cx, cy)) {
                car.setPosition(prevPos);
                car.applyBounce();
                return;
            }
        }

        // Check center position for road validity
        int px = static_cast<int>(pos.x / scale_);
        int py = static_cast<int>(pos.y / scale_);

        if (px < 0 || py < 0 || px >= scaledwidth_ || py >= scaledheight_) {
            car.applyBraking(0.9995f, dt * 60);
        } else if (!getBit(roadMask_, px, py)) {
            car.applyBraking(0.9995f, dt * 60);
        }
    }



    bool isPickupCollidingWithOBB(const Vector3& pickupPos, const OBB& box, float pickupRadius) {
        Vector3 toPickup = pickupPos - box.center;
        float xProj = toPickup.dot(box.right);
        float yProj = toPickup.dot(box.forward);

        return std::abs(xProj) <= box.halfExtents.x + pickupRadius &&
               std::abs(yProj) <= box.halfExtents.y + pickupRadius;
    }

    void handlePickupCollisions(CarLogic& car, float dt) {
        OBB carBox = car.getOrientedBoundingBox();

        auto nearbyIndices = pickupGrid_.queryNearby(car.getPosition(), 2);

        for (size_t idx : nearbyIndices) {
            Pickup& pickup = Pickups_[idx];
            if (!pickup.active) continue;

            if (isPickupCollidingWithOBB(pickup.position, carBox, pickup.radius)) {
                pickup.active = false;
                pickup.respawnTimer = 5.0f;
                inactivePickupIndices_.push_back(idx);
                car.setBoostFactor(car.boostFactor_ + 1);
            }
        }

        // Update inactive pickups and respawn them
        auto it = inactivePickupIndices_.begin();
        while (it != inactivePickupIndices_.end()) {
            Pickup& pickup = Pickups_[*it];
            pickup.respawnTimer -= dt;
            if (pickup.respawnTimer <= 0.0f) {
                pickup.active = true;
                it = inactivePickupIndices_.erase(it);
            } else {
                ++it;
            }
        }
    }



    void handleCheckPointing(CarLogic &car, float dt) {
        int px = static_cast<int>(car.getPosition().x / scale_);
        int py = static_cast<int>(car.getPosition().y / scale_);

        for (const auto finnishPos : CheckPointMap_[checkpoint]) {
            if (finnishPos.x == px and finnishPos.y == py) {
                std::cout << finnishPos.x << " " << finnishPos.y << std::endl;
                std::cout << px << " " << py << std::endl;
                checkpoint--;
                std::cout << "FINISHED the thingu thango" << 255-checkpoint << std::endl;
                break;
            }
        }
    }

    void setBit(std::vector<uint8_t> &mask, int x, int y) {
        if (x < 0 || y < 0 || x >= scaledwidth_ || y >= scaledheight_) {
            throw std::out_of_range(
                ("Invalid index in RaceGame::setBit: " + std::to_string(x) + "," + std::to_string(y)));
        }
        int idx = y * scaledwidth_ + x;
        mask[idx / 8] |= (1 << (idx % 8));
    }

    int getBit(const std::vector<uint8_t> &mask, int x, int y) {
        if (x < 0 || y < 0 || x >= scaledwidth_ || y >= scaledheight_) {
            throw std::out_of_range(
                ("Invalid index in RaceGame::getBit: " + std::to_string(x) + "," + std::to_string(y)));
        }
        int idx = y * scaledwidth_ + x;
        return mask[idx / 8] & (1 << (idx % 8));
    }

    std::pair<Vector3, float> getStartPositionAndRotation() {
        // Search the map for the first pixel where a road bit is set
        for (int y = 0; y < scaledheight_; ++y) {
            for (int x = 0; x < scaledwidth_; ++x) {
                if (getBit(roadMask_, x, y)) {
                    float startX = std::max(100.0f, (static_cast<float>(x) + 0.5f) * scale_);
                    float startY = std::max(100.0f, (static_cast<float>(y) + 0.5f) * scale_);

                    Vector3 startPos{startX, startY, 0};

                    float startRot = 0.0f;
                    std::cout << "Car got start position: " << startPos.x << " . " << startPos.z << std::endl;
                    return {startPos, startRot};
                }
            }
        }

        Vector3 fallbackPos{
            static_cast<float>(scaledwidth_ * scale_) / 2.0f + 100.f,
            static_cast<float>(scaledheight_ * scale_) / 2.0f + 100.f,
            0.0f,
        };

        std::cerr << "Warning: No road found in roadMask_! Falling back to map center." << std::endl;
        return {fallbackPos, 0.0f};
    }
};


#endif
