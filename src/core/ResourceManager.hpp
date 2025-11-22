//
// Created by Mella on 21.10.2025.
//

#// ResourceManager.hpp
#ifndef CARSIM_RESOURCEMANAGER_HPP
#define CARSIM_RESOURCEMANAGER_HPP

#include <threepp/threepp.hpp>
#include <unordered_map>
#include <memory>

// sketch — adapt to threepp API names in your project
class ResourceManager {
public:
    threepp::OBJLoader objLoader;
    std::unordered_map<std::string, std::shared_ptr<threepp::Group>> prototypes;

    std::shared_ptr<threepp::Group> loadPrototype(const std::string &path) {
        auto it = prototypes.find(path);
        if (it != prototypes.end()) return it->second;
        auto g = objLoader.load(path, true);
        computeBoundingForGroup(g.get());
        prototypes[path] = g;
        return g;
    }

    std::shared_ptr<threepp::Group> cloneInstance(const std::string &path) {
        auto proto = loadPrototype(path);
        return std::dynamic_pointer_cast<threepp::Group>(proto->clone(true));
    }

private:
    static void computeBoundingForGroup(const threepp::Object3D* obj) {
        if (!obj) return;

        if (auto mesh = dynamic_cast<const threepp::Mesh*>(obj)) {
            if (mesh->geometry()) {
                // geometry is a shared_ptr or raw pointer depending on API; adapt if needed
                mesh->geometry()->computeBoundingSphere();
            }
        }

        for (const auto* child : obj->children) {
            computeBoundingForGroup(child);
        }
    }
};


#endif //CARSIM_RESOURCEMANAGER_HPP
