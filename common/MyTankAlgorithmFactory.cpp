#include "MyTankAlgorithmFactory.h"
#include "DefensiveTankAlgorithm.h"
#include "OffensiveTankAlgorithm.h"

std::unique_ptr<TankAlgorithm> MyTankAlgorithmFactory::create(int /*player_index*/, int tank_index) const {
    // Create defensive tanks for even indices (tank_index % 2 == 0)
    // Create offensive tanks for odd indices (tank_index % 2 == 1)
    if (tank_index % 2 == 0) {
        return std::make_unique<DefensiveTankAlgorithm>();
    } else {
        return std::make_unique<OffensiveTankAlgorithm>();
    }
} 