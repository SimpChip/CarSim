//
// Created by Mella on 18.10.2025.
//

#include "RaceGame.hpp"
#include "iostream"
#include "threepp/threepp.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "cmake-build-debug-mingw/_deps/threepp-src/src/external/nlohmann/nlohmann/json.hpp"


using Vector3 = threepp::Vector3;

RaceGame::RaceGame(const std::string& filename)
{
    std::string meta_filename = "C:/Users/Mella/Dev/CLionProjects/CarSimV0/resources/maps/"+ filename + "M.png";
    int nChannels;
    unsigned char* data = stbi_load(meta_filename.c_str(), &width, &height, &nChannels, 3);
    if (!data) {
        std::cerr << "Failed to load map meta data image: " << meta_filename << std::endl;
        std::cerr << "stb_image error: " << stbi_failure_reason() << std::endl;
        return;
    }
    std::cout << "Loaded map meta data image: " << width << std::endl;
    roadMask.resize((width * height + 7) / 8, 0);
    obstacleMask.resize((width * height + 7) / 8, 0);
    for (int z = 0; z < height; ++z)
    {
        for (int x = 0; x < width; ++x)
        {
            int idx = (z * width + x) * 3;
            unsigned char r = data[idx];
            unsigned char g = data[idx+1];
            unsigned char b = data[idx+2];

            // Road mask (R channel)
            if (r > 250) setBit(roadMask, x, z);
            // Obstacle mask (G channel)
            if (g > 250) setBit(obstacleMask, x, z);

            // Checkpoint (B channel)
            if (b > 250) {std::cerr << "checkpoint" <<  std::endl;};
        }
    }

    stbi_image_free(data);
}
void RaceGame::handleMapCollisions(CarLogic& car)
{
    int px = static_cast<int>(car.getPosition().x);
    int pz = static_cast<int>(car.getPosition().z);
    auto bitSet = getBit(roadMask, px, pz);
    std::cout << "x=" <<px << "x=" <<pz  << std::endl;
    std::cout << "obstaclemask:" << (bitSet ? "true" : "false") << std::endl;
    if (!getBit(roadMask, px, pz)) {
        // Offroad: apply friction or slow down
        car.applyBraking(0.95);
    }

    if (getBit(obstacleMask, px, pz)) {
        // Collision with wall or object: stop car or bounce
        auto vel = car.getVelocity();
        const Vector3 prevPos = car.getPreviousPosition();
        const Vector3 pos = car.getPosition();
        car.setPosition(prevPos);
        car.stopCar();
    }
}
inline void RaceGame::setBit(std::vector<uint8_t>& mask, int x, int y) {
    int idx = y * width + x;           // linear index of the pixel
    mask[idx / 8] |= (1 << (idx % 8)); // set the corresponding bit inside the byte
}
inline int RaceGame::getBit(const std::vector<uint8_t>& mask, int x, int y) {
    int idx = y * width + x;
    return mask[idx / 8] & (1 << (idx % 8));
}

std::pair<Vector3, float> RaceGame::getStartPositionAndRotation() const
{
    // Default position and rotation
    Vector3 defaultPos{500,0,500};
    float defaultRot = 0.0f;

    return {defaultPos, defaultRot};
}


