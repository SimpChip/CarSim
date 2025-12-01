#ifndef CARSIM_ISCENE_HPP
#define CARSIM_ISCENE_HPP

#include "types/ILogicSnapShot.hpp"
#include "threepp/renderers/GLRenderer.hpp"

class IScene {
public:
    virtual ~IScene() = default;
    virtual void update(ILogicSnapshot& snap, float dt) = 0;
    virtual void render(threepp::GLRenderer& renderer) = 0;
};

#endif //CARSIM_ISCENE_HPP