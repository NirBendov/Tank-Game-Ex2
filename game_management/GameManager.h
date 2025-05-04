#pragma once
#include <memory>
#include <cstddef>
#include <string>
#include "../common/Player.h"
#include "../common/PlayerFactory.h"
#include "../common/TankAlgorithm.h"
#include "../common/TankAlgorithmFactory.h"

class GameManager
{
private:
    unique_ptr<Player> playerOne;
    unique_ptr<Player> playerTwo;
public:
    GameManager(PlayerFactory &player_factory, TankAlgorithmFactory &algorithmFactory);
    ~GameManager() {}
    void readBoard(string fileMame);
    void run();
};
