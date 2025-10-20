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
    int nChannels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nChannels, 3);
    if (!data) {
        std::cerr << "Failed to load map image: " << filename << std::endl;
        std::cerr << "stb_image error: " << stbi_failure_reason() << std::endl;
        return;
    }

    raceMap.resize(height, std::vector<int>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int idx = (y * width + x) * 3;
            const unsigned char r = data[idx];
            const unsigned char g = data[idx+1];
            const unsigned char b = data[idx+2];

            // Map colors to tiles
            if (r == 255 && g == 255 && b == 255) raceMap[y][x] = 1;
            else if (r == 128 && g == 128 && b == 128) raceMap[y][x] = 2;
            else if (r == 255 && g == 0 && b == 0) raceMap[y][x] = 3;
            else if (r == 0 && b == 0)
            {
                if (g == 255)
                {
                    raceMap[y][x] = 4;
                }
            }
            else if (r == 0 && g == 0)
            {
                if (b > 0) {
                    checkpointMap[b].add({static_cast<float>(x), 0.0f, static_cast<float>(y)});
                    raceMap[y][x] = 2;
                }
            }
            else raceMap[y][x] = 0; // empty
        }
    }

    if (!checkpointMap.empty()) {
        std::cout << "Start: x=" << checkpointMap[255].points[0].x
                  << ", y=" << checkpointMap[255].points[0].y << std::endl;
        std::cout << "End:   x=" << checkpointMap[255].points[1].x
                  << ", y=" << checkpointMap[255].points[1].y << std::endl;
    }

    stbi_image_free(data);
}
std::pair<Vector3, float> RaceGame::getStartPositionAndRotation() const
{
    // Default position and rotation
    Vector3 defaultPos{0,0,0};
    float defaultRot = 0.0f;

    if (checkpointMap.empty()) return {defaultPos, defaultRot};


    // Midpoint
    Checkpoint finish = checkpointMap.at(255) ;
    Vector3 startPos = finish.midpoint();

    // Rotation (yaw) along the line from first to second point

    float rotation = finish.angle(); // rotation around y-axis

    return {startPos, rotation};
}


