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
#include "stb_image.h"
#include "cmake-build-debug-mingw/_deps/threepp-src/src/external/nlohmann/nlohmann/json.hpp"

using Vector3 = threepp::Vector3;

struct Checkpoint {
    std::array<Vector3, 2> points;
    int count = 0; // how many points added

    void add(const Vector3 &p) {
        if (count < 2) {
            points[count++] = p;
        }
    }

    [[nodiscard]] Vector3 midpoint() const {
        if (count < 2) return {0, 0, 0};
        return (points[0] + points[1]) / 2.0f;
    }

    [[nodiscard]] float angle() const {
        const float angle = std::atan2(points[1].z - points[0].z, points[1].x - points[0].x);
        return angle;
    }
};

struct ImageInfo {
    int width, height, channels;
    std::string name;
    unsigned char *data;
};

class MapLogic {
public:
    std::vector<uint8_t> roadMask_; // 1 bit per pixel
    std::vector<uint8_t> obstacleMask_;
    std::map<int, Checkpoint> checkpointMap_;
    int width_{};
    int height_{};
    int scaledwidth_{};
    int scaledheight_{};
    int scale_ = 1;
    std::string filename_;

    explicit MapLogic(const std::string &filename) : filename_(filename) {
        std::cout << "RaceGame starts loading all " << filename_ << "files" << std::endl;
        std::vector<ImageInfo> sectionMaps = loadImageData();

        if (sectionMaps.empty()) {
            std::cerr << "Error: No map sections loaded. Aborting constructor." << std::endl;
            return;
        }

        int maxSectionX = 0, maxSectionY = 0;
        int sectionPixelWidth = sectionMaps[0].width;
        int sectionPixelHeight = sectionMaps[0].height;

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

        size_t maskSizeBytes = (static_cast<size_t>(scaledwidth_) * scaledheight_ + 7) / 8;
        roadMask_.resize(maskSizeBytes, 0);
        obstacleMask_.resize(maskSizeBytes, 0);


        for (const auto &section: sectionMaps) {
            std::cout << "Reading and Storing filedata from file: " << section.name << std::endl;
            auto [sectionX, sectionY] = parseMapPosition(section.name);

            int sectionScaledWidth = section.width / scale_;
            int sectionScaledHeight = section.height / scale_;

            int offsetX = sectionX * sectionScaledWidth;
            int offsetY = sectionY * sectionScaledHeight;

            for (int y = 0; y < sectionScaledHeight; ++y) {
                for (int x = 0; x < sectionScaledWidth; ++x) {
                    int rSum = 0, gSum = 0;
                    for (int yy = 0; yy < scale_; ++yy) {
                        for (int xx = 0; xx < scale_; ++xx) {
                            int px = x * scale_ + xx;
                            int py = y * scale_ + yy;
                            int idx = (py * section.width + px) * 3; // *3 for RGB

                            if (idx + 1 < section.width * section.height * 3) {
                                rSum += section.data[idx];
                                gSum += section.data[idx + 1];
                            }
                        }
                    }

                    int divisor = scale_ * scale_;
                    int rAvg = rSum / divisor;
                    int gAvg = gSum / divisor;

                    int maskX = x + offsetX;
                    int maskY = y + offsetY;

                    if (rAvg > 228) setBit(roadMask_, maskX, maskY); // Red channel for Road
                    if (gAvg > 228) setBit(obstacleMask_, maskX, maskY); // Green channel for Obstacle
                }
            }
            stbi_image_free(section.data);
        }
        std::cout << "RaceGame is Created" << std::endl;
    }

    std::pair<int, int> parseMapPosition(const std::string &filename) {
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

    std::vector<ImageInfo> loadImageData() {
        std::vector<ImageInfo> images;
        const std::filesystem::path directoryPath{(std::string(DATA_DIR) + "/maps/" + filename_ + "/")};
        for (const auto &entry: std::filesystem::directory_iterator(directoryPath)) {
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

    void handleMapCollisions(CarLogic &car, float dt) {
        auto vel = car.getVelocity();

        const Vector3 prevPos = car.getPreviousPosition();
        const Vector3 pos = car.getPosition();

        int px = static_cast<int>(car.getPosition().x / scale_);
        int pz = static_cast<int>(car.getPosition().z / scale_);
        // std::cout <<"z" << pz << " ---> s" << scaledwidth_ << std::endl;
        // std::cout <<"x" << px << " ---> s" << scaledheight_ << std::endl;


        if (px < 0 or pz < 0 or px >= scaledwidth_ or pz >= scaledheight_) {
            std::cout << "OUT OF BOUNDS?!?!??" << std::endl;
            car.setPosition(prevPos);
            car.stopCar();
        } else {
            if (!getBit(roadMask_, px, pz)) {
                car.applyBraking(0.999, dt * 60);
            }

            if (getBit(obstacleMask_, px, pz)) {
                car.setPosition(prevPos);
                car.stopCar();
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
        // Search the scaled map for the first pixel where a road bit is set
        for (int y = 0; y < scaledheight_; ++y) {
            for (int x = 0; x < scaledwidth_; ++x) {
                if (getBit(roadMask_, x, y)) {
                    float startX = std::max(100.0f, (static_cast<float>(x) + 0.5f) * scale_);
                    float startZ = std::max(100.0f, (static_cast<float>(y) + 0.5f) * scale_);

                    Vector3 startPos{startX, 0.0f, startZ};

                    float startRot = 0.0f;
                    std::cout << "Car got start position: " << startPos.x << " . " << startPos.z << std::endl;
                    return {startPos, startRot};
                }
            }
        }

        Vector3 fallbackPos{
            static_cast<float>(scaledwidth_ * scale_) / 2.0f + 100.f,
            0.0f,
            static_cast<float>(scaledheight_ * scale_) / 2.0f + 100.f,
        };

        std::cerr << "Warning: No road found in roadMask_! Falling back to map center." << std::endl;
        return {fallbackPos, 0.0f};
    }
};


#endif
