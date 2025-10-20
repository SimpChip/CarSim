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
    roadMask.resize((width * height + 7) / 8, 0);
    obstacleMask.resize((width * height + 7) / 8, 0);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int idx = (y * width + x) * 3;
            unsigned char r = data[idx];
            unsigned char g = data[idx+1];
            unsigned char b = data[idx+2];

            // Road mask (R channel)
            if (r > 250) setBit(roadMask, x, y, width);
            // Obstacle mask (G channel)
            if (g > 250) setBit(obstacleMask, x, y, width);

            // Checkpoint (B channel)
            if (b > 250) {std::cerr << "checkpoint" <<  std::endl;};
        }
    }

    stbi_image_free(data);
}
inline void RaceGame::setBit(std::vector<uint8_t>& mask, int x, int y, int width) {
    int idx = y * width + x;           // linear index of the pixel
    mask[idx / 8] |= (1 << (idx % 8)); // set the corresponding bit inside the byte
}
inline int RaceGame::getBit(const std::vector<uint8_t>& mask, int x, int y, int width) {
    int idx = y * width + x;
    return mask[idx / 8] & (1 << (idx % 8));
}

std::pair<Vector3, float> RaceGame::getStartPositionAndRotation() const
{
    // Default position and rotation
    Vector3 defaultPos{0,0,0};
    float defaultRot = 0.0f;

    return {defaultPos, defaultRot};
}


