#pragma once
#include "Player.h"
#include "SatelliteView.h"
#include "TankAlgorithm.h"

class Player2 : public Player {
private:
    int player_index;
    size_t x;
    size_t y;
    size_t max_steps;
    size_t num_shells;

public:
    Player2(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells);
    void updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &satellite_view) override;
}; 