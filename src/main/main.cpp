
#include <threepp/threepp.hpp>
#include <GameLogic.hpp>
#include <GameManager.hpp>
#include <ctime>
#include <UIManager.hpp>

#define STB_IMAGE_IMPLEMENTATION


int main() {
    float globScale = 10.f;
    threepp::Canvas canvas("Race Instanced");
    threepp::GLRenderer renderer(canvas.size());
    renderer.setClearColor(threepp::Color::whitesmoke);
    GameManager manager;
    manager.attachListeners(canvas);
    UIManager ui(canvas.windowPtr());

    canvas.onWindowResize([&](const threepp::WindowSize& size) {
        renderer.setSize(size);
    });
    // struct MakerMouseListener : public threepp::MouseListener {
    //
    //     GameManager* manager;
    //     threepp::Canvas* canvas;
    //
    //     MakerMouseListener(GameManager* manager, threepp::Canvas* canvas)
    //         : manager(manager), canvas(canvas) {}
    //
    //     void onMouseDown(int button, const threepp::Vector2& pos) override {
    //         if (manager->state == AppState::MAKER && manager->getMapCreatorScene()) {
    //             std::cout << "CLICK" << pos << std::endl;
    //             manager->getMapCreatorScene()->onMouseClick(
    //                 pos.x, pos.y,
    //                 canvas->size().width(), canvas->size().height()
    //             );
    //         }
    //     }
    // };

    // MakerMouseListener mouse(&manager, &canvas);
    // canvas.addMouseListener(mouse);

    std::cout << "Started Game, Have fun!"<< std::endl;
    threepp::Clock clock;
    canvas.animate([&](){
        const float dt = clock.getDelta();
        manager.update(dt);
        manager.render(renderer);
        manager.handleUI(ui, dt);
   });

    return 0;
}