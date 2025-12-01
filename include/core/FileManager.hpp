#ifndef CARSIM_FILEMANAGER_HPP
#define CARSIM_FILEMANAGER_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <threepp/math/Vector3.hpp>
#include <threepp/objects/Group.hpp>

struct MapConfig {
    std::string name;
    std::string type;
    std::vector<threepp::Vector3> roadPoints;
    bool looped;
    int roadWidth;
    int width;
    int height;
    int checkpoints;
};


struct ModelVariant {
    std::string color;
    std::shared_ptr<threepp::Group> body;
};

struct ModelEntry {
    std::string name;
    std::vector<std::string> wheelPaths;
    std::unordered_map<std::string, std::string> colorBodies;
};

class FileManager {
public:
    static std::optional<MapConfig> loadMapConfig(const std::string& path);
    static bool saveMapConfig(const MapConfig& cfg, const std::string& path);
    static std::optional<std::vector<MapConfig>> loadAllMaps(const std::filesystem::path& path);
    static std::vector<ModelEntry> scanModels(const std::filesystem::path& root);
};

#endif //CARSIM_FILEMANAGER_HPP