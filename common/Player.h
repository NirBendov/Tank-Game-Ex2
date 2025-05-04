#pragma once
#include <cstddef>
#include "BattleInfo.h"
#include "TankAlgorithm.h"
#include "SatelliteView.h"

using namespace std;

class Player
{
private:
    
public:
    Player(int player_index, 
            size_t x, size_t y,
            size_t max_steps, size_t num_shells);
    virtual ~Player() {}
    virtual void updateTankWithBattleInfo(TankAlgorithm &tank,
                                        SatelliteView &Satellite_view) = 0;
};
