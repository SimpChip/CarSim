#include "core/GameManager.hpp"
#include "core/UiManager.hpp"

int main() {
    threepp::Canvas canvas("Race Instanced");
    threepp::GLRenderer renderer(canvas.size());
    renderer.setClearColor(threepp::Color::whitesmoke);
    GameManager manager;
    manager.attachListeners(canvas);
    UIManager ui(canvas.windowPtr());

    canvas.onWindowResize([&](const threepp::WindowSize& size) {
        renderer.setSize(size);
    });
    threepp::Clock clock;
    canvas.animate([&](){
        const float dt = clock.getDelta();
        manager.update(dt);
        manager.render(renderer);
        manager.handleUI(ui, dt);
   });
    return 0;
}