#ifndef CARSIM_RESOURCEMANAGER_HPP
#define CARSIM_RESOURCEMANAGER_HPP

#include "threepp/objects/Mesh.hpp"
#include "FileManager.hpp"

class ResourceManager {
public:
    ResourceManager() {
        loadAllMaps();
        loadAllModels();
        loadAllTextures();
    };

    void loadAllTextures();
    void loadAllModels();
    void loadAllMaps();

    std::shared_ptr<threepp::Group> loadOBJ(const std::string &path);
    std::shared_ptr<threepp::Texture> loadTexture(const std::string &path);

    std::shared_ptr<threepp::Texture> getTexture(const std::string &name) const;
    std::unordered_map<std::string, std::shared_ptr<MapConfig>> getMapConfigs() const;
    std::shared_ptr<MapConfig> getMapConfig(const std::string &name) const;

    std::vector<std::string> getMapNames() const;
    std::vector<std::string> getModelNames() const;

    const ModelData& getModel(const std::string& carName, const std::string& color) const;

    std::shared_ptr<threepp::Group> cloneInstance(const std::string &path);

    std::shared_ptr<threepp::Group> cloneCarBody(const std::string& carName,
                                             const std::string& color) const;

    std::vector<std::shared_ptr<threepp::Group>> cloneCarWheels(const std::string& carName,
                                                                const std::string& color) const;
    void clear() {
        prototypes_.clear();
        textures_.clear();
        mapConfigs_.clear();
    }

private:

    std::unordered_map<std::string, std::shared_ptr<threepp::Group>> prototypes_;
    std::unordered_map<std::string, std::shared_ptr<threepp::Texture>> textures_;
    std::unordered_map<std::string, std::shared_ptr<MapConfig>> mapConfigs_;

    std::unordered_map<std::string,
    std::unordered_map<std::string, ModelData>> models_;

    static void computeBoundingForGroup(const threepp::Object3D* obj) {
        if (!obj) return;

        if (const auto mesh = dynamic_cast<const threepp::Mesh*>(obj)) {
            if (mesh->geometry()) {
                mesh->geometry()->computeBoundingSphere();
            }
        }

        for (const auto* child : obj->children) {
            computeBoundingForGroup(child);
        }
    }
};

#endif //CARSIM_RESOURCEMANAGER_HPP