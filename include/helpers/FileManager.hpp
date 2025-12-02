#ifndef CARSIM_FILEMANAGER_HPP
#define CARSIM_FILEMANAGER_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "types/mapConfig.hpp"
#include "types/model.hpp"



class FileManager {
public:
    static std::optional<MapConfig> loadMapConfig(const std::string& path);
    static bool saveMapConfig(const MapConfig& cfg, const std::string& path);
    static std::optional<std::vector<MapConfig>> loadAllMaps(const std::filesystem::path& path);
    static std::vector<ModelEntry> scanModels(const std::filesystem::path& root);
};

#endif //CARSIM_FILEMANAGER_HPP