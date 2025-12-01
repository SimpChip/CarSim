
#include "core/ResourceManager.hpp"
#include "iostream"
#include "threepp/loaders/TextureLoader.hpp"


//Textures
void ResourceManager::loadAllTextures() {
    const auto root = std::filesystem::path(std::string(DATA_DIR) + "/textures");
    if (!std::filesystem::exists(root)) {
        std::cerr << "Texture directory not found: " << root << "\n";
        return;
    }
    for (auto& file : std::filesystem::directory_iterator(root)) {
        if (file.is_regular_file()) {
            if (file.path().extension() == ".png" || file.path().extension() == ".jpg") {
                loadTexture(file.path().string());
            }
        }
    }
}
std::shared_ptr<threepp::Texture> ResourceManager::loadTexture(const std::string &path) {
    threepp::TextureLoader tl;
    std::string key = std::filesystem::path(path).stem().string();

    auto [it, inserted] = textures_.try_emplace(key, nullptr);
    if (!inserted) return it->second;

    it->second = tl.load(path);
    return it->second;
}

std::shared_ptr<threepp::Texture> ResourceManager::getTexture(const std::string &name) const {
    const auto it = textures_.find(name);
    if (it != textures_.end()) {
        return it->second;
    }
    std::cerr << "Texture not found: " << name << "\n";
    return nullptr;
}

//Objects
void ResourceManager::loadAllModels() {
    const auto root = std::filesystem::path(std::string(std::string(DATA_DIR) + "/models"));
    auto entries = FileManager::scanModels(root);
    for (auto&[name, wheelPaths, colorBodies] : entries) {
        std::vector<std::shared_ptr<threepp::Group>> wheels;
        for (auto& w : wheelPaths) {
            wheels.push_back(loadOBJ(w));
        }
        for (auto& [color, bodyPath] : colorBodies) {
            //std::cout << bodyPath << std::endl;
            const auto body = loadOBJ(bodyPath);
            models_[name][color] = {body, wheels};
        }
    }
}

const ModelData& ResourceManager::getModel(const std::string& carName,
                                           const std::string& color) const {
    const auto itCar = models_.find(carName);
    if (itCar == models_.end()) {
        throw std::runtime_error("Model not found: " + carName);
    }

    const auto itColor = itCar->second.find(color);
    if (itColor == itCar->second.end()) {
        throw std::runtime_error(
            "Color " + color + " not found for model " + carName
        );
    }

    return itColor->second;
}

std::shared_ptr<threepp::Group> ResourceManager::cloneInstance(const std::string &path) {
    const auto proto = loadOBJ(path);
    return std::dynamic_pointer_cast<threepp::Group>(proto->clone(true));
}

std::shared_ptr<threepp::Group> ResourceManager::cloneCarBody(
    const std::string& carName,
    const std::string& color
) const {

    const auto&[body, wheels] = getModel(carName, color);
    return std::dynamic_pointer_cast<threepp::Group>(body->clone(true));
}
std::vector<std::shared_ptr<threepp::Group>>
ResourceManager::cloneCarWheels(const std::string& carName,
                                const std::string& color) const
{
    const auto&[body, wheels] = getModel(carName, color);
    std::vector<std::shared_ptr<threepp::Group>> cloned;
    cloned.reserve(wheels.size());
    for (auto& w : wheels) {
        cloned.push_back(
            std::dynamic_pointer_cast<threepp::Group>(w->clone(true))
        );
    }

    return cloned;
}

std::shared_ptr<threepp::Group> ResourceManager::loadOBJ(const std::string &path) {
    threepp::OBJLoader ol;
    const auto it = prototypes_.find(path);
    if (it != prototypes_.end()) return it->second;
    auto g = ol.load(path, true);
    if (!g) {
        std::cerr << "Failed to load OBJ: " << path << "\n";
        return nullptr;
    }
    computeBoundingForGroup(g.get());
    prototypes_[path] = g;
    return g;
}

std::vector<std::string> ResourceManager::getModelNames() const {
    if (models_.empty()) {
        return {"No models available"};
    }
    std::vector<std::string> names;
    names.reserve(models_.size());
    for (const auto &carName: models_ | std::views::keys) {
        names.push_back(carName);
    }

    return names;
}


//Maps
void ResourceManager::loadAllMaps() {
    auto configs = FileManager::loadAllMaps(std::filesystem::path(std::string(DATA_DIR) + "/mapConfigs"));
    if (!configs) {
        std::cerr << "No map configs found in " << (std::filesystem::path(std::string(DATA_DIR) + "/mapConfigs")) << "\n";
        return;
    }
    for (auto& cfg : *configs) {
        mapConfigs_[cfg.name] = std::make_shared<MapConfig>(cfg);
    }
}

std::unordered_map<std::string, std::shared_ptr<MapConfig>> ResourceManager::getMapConfigs() const{
    return mapConfigs_;
}

std::shared_ptr<MapConfig> ResourceManager::getMapConfig(const std::string &name) const {
    const auto it = mapConfigs_.find(name);
    if (it != mapConfigs_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> ResourceManager::getMapNames() const {
    if (mapConfigs_.empty()) {
        return {"No maps available"};
    }
    std::vector<std::string> names;
    names.reserve(mapConfigs_.size());
    for (const auto &name: mapConfigs_ | std::views::keys) {
        names.push_back(name);
    }
    return names;
}




