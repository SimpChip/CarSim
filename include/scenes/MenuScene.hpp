#ifndef CARSIM_MENUSCENE_HPP
#define CARSIM_MENUSCENE_HPP

#include "iostream"
#include "helpers/ResourceManager.hpp"

class MenuScene {
    public:
    explicit MenuScene(const ResourceManager& resources) : scene_(threepp::Scene::create()) {
        scene_->background = threepp::Color::lightblue;
        camera_ = threepp::OrthographicCamera::create(-50, 50, 50, -50, 0.1f, 1000.f);
        camera_->position.set(0, 0, 100);
        camera_->lookAt(0, 0, 0);

        const auto texture = resources.getTexture("wall");
        const auto backgroundMaterial = threepp::MeshBasicMaterial::create();
        if (!texture) {
            std::cerr << "Failed to load background texture for Menu Scene\n";
            backgroundMaterial->color = threepp::Color::beige;
        } else {
            backgroundMaterial->side = threepp::Side::Double;
            backgroundMaterial->map = texture;
            backgroundMaterial->color = threepp::Color::white;
        }
        const auto plane = threepp::Mesh::create(
            threepp::PlaneGeometry::create(100, 100),
            backgroundMaterial
        );
        plane->position = threepp::Vector3(0,0,-1);
        scene_->add(plane);
    }

    void render(threepp::GLRenderer &renderer) const {
        if (scene_ && camera_) {
            renderer.render(*scene_, *camera_);
        }
    };

private:
    std::shared_ptr<threepp::Scene> scene_;
    std::shared_ptr<threepp::OrthographicCamera> camera_;
};

#endif //CARSIM_MENUSCENE_HPP