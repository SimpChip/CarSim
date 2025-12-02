#include "helpers/FileManager.hpp"
#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"
#include "threepp/math/Vector3.hpp"

std::optional<std::vector<MapConfig>> FileManager::loadAllMaps(const std::filesystem::path& path) {
    std::vector<MapConfig> configs;
    try {
        if (!std::filesystem::exists(path)) {
            return std::nullopt;
        }

        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                if (auto cfg = loadMapConfig(entry.path().string())) {
                    configs.push_back(*cfg);
                }
            }
        }
    } catch (...) {
        return std::nullopt;
    }

    if (configs.empty()) {
        return std::nullopt;
    }
    return configs;
}

std::optional<MapConfig> FileManager::loadMapConfig(const std::string &path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) return std::nullopt;

        nlohmann::json j;
        file >> j;

        MapConfig cfg;
        cfg.name        = j.value("name", "Unnamed");
        cfg.type        = j.value("type", "normal");
        if (j.contains("roadPoints")) {
            for (const auto& arr : j["roadPoints"]) {
                threepp::Vector3 v;
                v.x = arr[0].get<float>();
                v.y = arr[1].get<float>();
                v.z = arr[2].get<float>();
                cfg.roadPoints.push_back(v);
            }
        }
        cfg.looped = j.value ("looped", true);
        cfg.roadWidth = j.value("roadWidth", 0);
        cfg.width       = j.value("width", 100);
        cfg.height      = j.value("height", 100);
        cfg.checkpoints = j.value("checkpoints", 0);

        return cfg;
    } catch (...) {
        return std::nullopt;
    }
}

bool FileManager::saveMapConfig(const MapConfig &cfg, const std::string &path) {
    try {
        nlohmann::json j;
        j["name"]   = cfg.name;
        j["width"]  = cfg.width;
        j["height"] = cfg.height;
        for (const auto& p : cfg.roadPoints) {
            j["roadPoints"].push_back({p.x, p.y, p.z});
        }
        j["checkpoints"] = cfg.checkpoints;
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << j.dump(4);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<ModelEntry> FileManager::scanModels(const std::filesystem::path& root) {
    std::vector<ModelEntry> entries;

    if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
        return entries;
    }
    for (auto& carDir : std::filesystem::directory_iterator(root)) {
        if (!carDir.is_directory()) continue;

        ModelEntry entry;
        entry.name = carDir.path().filename().string();
        //std::cout << "CarPath is" << carDir.path() << std::endl;
        //std::cout << "Entry Name is: " << entry.name << std::endl;

        auto wheelsDir = carDir.path() / "wheels";
        if (std::filesystem::exists(wheelsDir) && std::filesystem::is_directory(wheelsDir)) {
            for (auto& wheelFile : std::filesystem::directory_iterator(wheelsDir)) {
                if (wheelFile.path().extension() == ".obj") {
                    entry.wheelPaths.push_back(wheelFile.path().string());
                }
            }
        }

        for (auto& colorDir : std::filesystem::directory_iterator(carDir)) {
            if (!colorDir.is_directory()) continue;
            if (colorDir.path().filename() == "wheels") continue;

            std::string colorName = colorDir.path().filename().string();
            for (auto& file : std::filesystem::directory_iterator(colorDir)) {
                if (file.path().extension() == ".obj") {
                    auto normalizedPath = file.path().lexically_normal().string();
                    //std::cout << colorName << ": " << normalizedPath << std::endl;
                    entry.colorBodies[colorName] = normalizedPath;
                    break;
                }
            }
        }
        entries.push_back(entry);
    }

    return entries;
}



