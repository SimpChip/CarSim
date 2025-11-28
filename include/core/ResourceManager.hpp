//
// Created by Mella on 27.11.2025.
//

#ifndef CARSIM_RESOURCEMANAGER_HPP
#define CARSIM_RESOURCEMANAGER_HPP

#include "threepp/loaders/OBJLoader.hpp"
#include "threepp/loaders/TextureLoader.hpp"
#include "threepp/objects/Mesh.hpp"

class ResourceManager {
public:
    threepp::OBJLoader objLoader;
    threepp::TextureLoader textureLoader;
    std::unordered_map<std::string, std::shared_ptr<threepp::Group>> prototypes;
    std::unordered_map<std::string, std::shared_ptr<threepp::Texture>> textures;

    std::shared_ptr<threepp::Group> loadPrototype(const std::string &path) {
        const auto it = prototypes.find(path);
        if (it != prototypes.end()) return it->second;
        auto g = objLoader.load(path, true);
        computeBoundingForGroup(g.get());
        prototypes[path] = g;
        return g;
    }

    std::shared_ptr<threepp::Texture> loadTexture(const std::string &path) {
        auto [it, inserted] = textures.try_emplace(path, nullptr);
        if (!inserted) return it->second;
        it->second = textureLoader.load(path, true);
        return it->second;
    }

    std::shared_ptr<threepp::Group> cloneInstance(const std::string &path) {
        auto proto = loadPrototype(path);
        return std::dynamic_pointer_cast<threepp::Group>(proto->clone(true));
    }

    void clear() {
        prototypes.clear();
        textures.clear();
    }

private:
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