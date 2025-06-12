#include "Player2.h"
#include "SatelliteBattleInfo.h"
#include <iostream>

Player2::Player2(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
    : Player(player_index, x, y, max_steps, num_shells),
      player_index(player_index), x(x), y(y), max_steps(max_steps), num_shells(num_shells) {}

void Player2::updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &satellite_view) {
    std::cout << "Player2: Updating battle info for tank" << std::endl;
    SatelliteBattleInfo battle_info(&satellite_view, player_index);
    battle_info.updateBoard();
    std::cout << "Player2: Battle info updated, sending to tank algorithm" << std::endl;
    tank.updateBattleInfo(battle_info);
    std::cout << "Player2: Tank algorithm updated with battle info" << std::endl;
} 