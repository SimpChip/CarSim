#ifndef CARSIM_EVENTS_HPP
#define CARSIM_EVENTS_HPP

#include "types/types.hpp"
#include "threepp/input/KeyListener.hpp"


struct KeyPressedEvent {
    const threepp::KeyEvent evt;
};

struct KeyReleasedEvent {
    const threepp::KeyEvent evt;
};

struct RoadExitEvent {
    int entityId;
    float lateral;
    int side;
};

struct RoadEnterEvent {
    int entityId;
};

struct MapExitEvent {
    int entityId;
};

struct CollisionEnterEvent {
    int idA, idB;
    CollisionType typeA, typeB;
};
struct CollisionExitEvent {
    int idA, idB;
    CollisionType typeA, typeB;
};

struct StartRaceEvent {
    std::string map;
    std::string model;
    int aiCount;
};

struct StartMapMakerEvent { };

struct PauseEvent { };
struct ResumeEvent { };

#endif //CARSIM_EVENTS_HPP