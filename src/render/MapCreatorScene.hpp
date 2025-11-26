//
// Created by Mella on 22.11.2025.
//

#ifndef CARSIM_MAPCREATORSCENE_HPP
#define CARSIM_MAPCREATORSCENE_HPP

#include <threepp/threepp.hpp>
#include "IScene.hpp"
#include "MapCreatorLogic.hpp"

class MapCreatorScene : public IScene {
public:
    explicit MapCreatorScene(MapCreatorLogic& logic)
        : logic_(logic),
          scene_(threepp::Scene::create()) {

        scene_->background = threepp::Color::black;

        const auto mat = threepp::MeshBasicMaterial::create();
        mat->side = threepp::Side::Double;
        mat->color = threepp::Color::lightblue;

        plane_ = threepp::Mesh::create(
            threepp::PlaneGeometry::create(logic_.getMapSize(), logic_.getMapSize()), mat
        );
        plane_->position.set(logic_.getMapSize()/2.f, logic_.getMapSize()/2.f, -1);
        scene_->add(plane_);

        camera_ = threepp::OrthographicCamera::create(
            -logic_.getMapSize()/2.f, logic_.getMapSize()/2.f,
            logic_.getMapSize()/2.f, -logic_.getMapSize()/2.f,
            0.1f, 100000.f
        );
        camera_->position.set(logic_.getMapSize()/2.f, logic_.getMapSize()/2.f, 200);
        camera_->lookAt(logic_.getMapSize()/2.f, logic_.getMapSize()/2.f, 0);
    }

    void update(float dt) {
        updateRoadLine();
    }

    void render(threepp::GLRenderer& renderer) override {
        renderer.render(*scene_, *camera_);
    }

    void onMouseClick(float mouseX, float mouseY, int screenW, int screenH) {
        float ndcX = (mouseX / screenW) * 2 - 1;
        float ndcY = -(mouseY / screenH) * 2 + 1;
        threepp::Raycaster raycaster;
        raycaster.setFromCamera({ndcX, ndcY}, *camera_);

        const auto intersects = raycaster.intersectObject(*plane_);
        if (!intersects.empty()) {
            const auto hit = intersects[0].point;
            logic_.addPoint(hit);
            addPointMarker(hit.x, hit.y);
        }
    }

    void resizeCamera(int width, int height) {
        const float aspect = static_cast<float>(width) / static_cast<float>(height);
        const float halfSize = logic_.getMapSize() / 2.f;

        if (aspect >= 1.0f) {
            camera_->left   = -halfSize * aspect;
            camera_->right  =  halfSize * aspect;
            camera_->top    =  halfSize;
            camera_->bottom = -halfSize;
        } else {
            camera_->left   = -halfSize;
            camera_->right  =  halfSize;
            camera_->top    =  halfSize / aspect;
            camera_->bottom = -halfSize / aspect;
        }

        camera_->position.set(logic_.getMapSize()/2.f, logic_.getMapSize()/2.f, 200);
        camera_->lookAt(logic_.getMapSize()/2.f, logic_.getMapSize()/2.f, 0);
        camera_->updateProjectionMatrix();
    }

private:
    MapCreatorLogic& logic_;
    std::shared_ptr<threepp::Scene> scene_;
    std::shared_ptr<threepp::Mesh> plane_;
    std::shared_ptr<threepp::OrthographicCamera> camera_;
    std::vector<std::shared_ptr<threepp::Object3D>> pointMarkers_;
    std::shared_ptr<threepp::Line> roadLine_;

    void addPointMarker(float x, float y, const threepp::Color& color = threepp::Color::red) {
        const auto mat = threepp::MeshBasicMaterial::create();
        mat->side = threepp::Side::Double;
        mat->color = color;
        const float markerRadius = std::pow(logic_.getMapSize()/100.f, 0.9f);
        const auto geo = threepp::CircleGeometry::create(markerRadius, 16);
        const auto marker = threepp::Mesh::create(geo, mat);

        marker->position.set(x, y, 1);
        scene_->add(marker);
        pointMarkers_.push_back(marker);
    }

    void updateRoadLine() {
        const auto& points = logic_.getPoints();
        if (points.empty()) return;

        std::vector<threepp::Vector3> drawPoints = points;
        if (points.size() > 2 && logic_.isLooped()) {
            drawPoints.push_back(points.front());
        }

        auto geo = threepp::BufferGeometry::create();
        geo->setFromPoints(drawPoints);

        auto mat = threepp::LineBasicMaterial::create();
        mat->color = threepp::Color::red;

        auto line = threepp::Line::create(geo, mat);

        if (roadLine_) {
            scene_->remove(*roadLine_);
        }
        roadLine_ = line;
        scene_->add(roadLine_);
    }
};

#endif // CARSIM_MAPCREATORSCENE_HPP
