#include <iostream>
#include <math.h>

#include "cmake-build-debug-mingw/_deps/threepp-src/src/external/nlohmann/nlohmann/json.hpp"
#include "threepp/threepp.hpp"
#include "threepp/canvas/Monitor.hpp"
using namespace threepp;

int main()
{

    Canvas canvas("Car Sim" , {{"aa", 6}});
    GLRenderer renderer(canvas.size());
    renderer.autoClear = false;

    Scene scene;
    scene.background = Color::aliceblue;

    PerspectiveCamera camera(60, canvas.aspect(), 0.1f, 1000);
    camera.position.set(0, 5, 10);
    camera.layers.enableAll();

    OrbitControls controls(camera, canvas);

    auto grid = GridHelper::create(100, 100);
    scene.add(grid);

    // //ground
    // float ground_size = 1000;
    // auto geometry_ground = BoxGeometry::create(ground_size, 0.1, ground_size);
    // auto material_ground = MeshBasicMaterial::create();
    // material_ground->color = Color::brown;
    // auto mesh_ground = Mesh::create(geometry_ground, material_ground);
    // mesh_ground->position = Vector3(0, -1, 0);
    //
    // scene.add(mesh_ground);

    //car
    auto car_group = Group::create();
    auto geometry = BoxGeometry::create(1, 0.8, 2);
    auto material_car = MeshBasicMaterial::create();
    material_car->color = Color::red;

    auto mesh = Mesh::create(geometry, material_car);
    mesh->position.set(0, 0, 0);

    car_group->add(mesh);

    auto wheel_group = Group::create();
    auto wheel_geometry = CylinderGeometry::create(0.4, 0.4, 0.1);
    auto material_wheel = MeshBasicMaterial::create();
    material_wheel->color = Color::green;
    auto wheel_fl = Mesh::create(wheel_geometry, material_wheel);
    wheel_fl->position.set(-0.4, -0.4, -1);
    auto wheel_fr = Mesh::create(wheel_geometry, material_wheel);
    wheel_fr->position.set(-0.4, 0.4, -1);
    auto wheel_bl = Mesh::create(wheel_geometry, material_wheel);
    wheel_bl->position.set(-0.4, -0.4, 1);
    auto wheel_br = Mesh::create(wheel_geometry, material_wheel);
    wheel_br->position.set(-0.4, 0.4, 1);
    wheel_group->add(wheel_fl);
    wheel_group->add(wheel_fr);
    wheel_group->add(wheel_bl);
    wheel_group->add(wheel_br);
    wheel_group->rotation.z = M_PI/2;
    car_group->add(wheel_group);

    scene.add(car_group);

    bool movingForward = false;
    bool turningLeft = false;
    bool turningRight = false;
    bool movingBack = false;

    float velocity = 0.0f;
    float acceleration = 3.0f;
    float friction = 6.0f;
    float rotationSpeed = 2.0f;

    Vector3 boxPosition{0,0.75,0};
    float boxRotation = 0.0f;


    KeyAdapter adapter(KeyAdapter::Mode::KEY_PRESSED, [&](KeyEvent evt) {
        if (evt.key == Key::W) { movingForward = true; movingBack = false; }
        if (evt.key == Key::A) turningLeft = true;
        if (evt.key == Key::D) turningRight = true;
        if (evt.key == Key::S) { movingForward = false; movingBack = true; }
    });

    KeyAdapter adapterRelease(KeyAdapter::Mode::KEY_RELEASED, [&](KeyEvent evt) {
        if (evt.key == Key::W) movingForward = false;
        if (evt.key == Key::A) turningLeft = false;
        if (evt.key == Key::D) turningRight = false;
        if (evt.key == Key::S) movingBack = false;
    });

    canvas.addKeyListener(adapter);
    canvas.addKeyListener(adapterRelease);

    Clock clock;
    float time = 0.0f;
    canvas.animate([&]
    {
        float dt = clock.getDelta();
        time += dt;

        static float movementDirection = boxRotation;
        float driftStrength = 6.0f;

        if (turningLeft)
        {
            boxRotation += rotationSpeed * dt;
            for (auto wheel : wheel_group->children)
            {
                wheel->rotation.x = -rotationSpeed * dt * 10;
            }

        }
        if (turningRight)
        {
            boxRotation -= rotationSpeed * dt;
            for (auto wheel : wheel_group->children)
            {
                wheel->rotation.x = rotationSpeed * dt * 10;
            }

        };

        if (!turningRight and !turningLeft)
        {
            for (auto wheel : wheel_group->children)
            {
                wheel->rotation.x = 0;
            }
        }

        if (movingForward) {
            for (auto wheel : wheel_group->children)
            {
                wheel->rotation.y -= rotationSpeed * dt * 10;
            }

            if (velocity < 0) velocity += acceleration * dt * friction;
            else velocity += acceleration * dt;
        }
        if (movingBack) {
            for (auto wheel : wheel_group->children)
            {
                wheel->rotation.y += rotationSpeed * dt * 10;
            }
            if (velocity > 0) velocity -= acceleration * dt * friction;
            else velocity -= acceleration * dt * 0.5;
        }
        if (!movingForward and !movingBack) {
            // Apply friction opposite to current velocity
            if (velocity > 0.0f) {

                velocity -= friction * dt;
                if (velocity < 0.0f) velocity = 0.0f;  // clamp
            }
            else if (velocity < 0.0f) {

                velocity += friction * dt;
                if (velocity > 0.0f) velocity = 0.0f;  // clamp
            }
        }
        float angleDiff = boxRotation - movementDirection;

        while (angleDiff > 3.14159f) angleDiff -= 6.28318f;
        while (angleDiff < -3.14159f) angleDiff += 6.28318f;

        movementDirection += angleDiff * dt * driftStrength;

        boxPosition.x -= sin(movementDirection) * velocity * dt;
        boxPosition.z -= cos(movementDirection) * velocity * dt;

        car_group->position = boxPosition;
        car_group->rotation.y = boxRotation;

        camera.position = boxPosition - Vector3(
            -sin(boxRotation) * 6,  // behind the car in local space
            -3,                      // height
            -cos(boxRotation) * 6
        );
        camera.lookAt(boxPosition);
        renderer.render(scene, camera);
    });
}