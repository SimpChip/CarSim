
#ifndef CARSIM_ILOGIC_HPP
#define CARSIM_ILOGIC_HPP

#include <memory>
#include "types/ILogicSnapShot.hpp"
#include "types/SnapShots.hpp"

class ILogic {
public:
    virtual ~ILogic() = default;
    virtual void update(float dt) = 0;
    virtual GameSnapShot snapshot() const = 0;
};

#endif //CARSIM_ILOGIC_HPP