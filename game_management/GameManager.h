#pragma once
#include <memory>
#include <cstddef>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "../common/Player.h"
#include "../common/PlayerFactory.h"
#include "../common/TankAlgorithm.h"
#include "../common/TankAlgorithmFactory.h"
#include "../constants/BoardConstants.h"

class GameManager
{
private:
    struct {
        string mapName;
        size_t maxStep;
        size_t numShells;
        size_t rows;
        size_t columns;
    } gameData;
    vector<vector<char>> board;
    unique_ptr<Player> playerOne;
    unique_ptr<Player> playerTwo;

    size_t extractVal(ifstream &f, string param);
    void buildBoard(ifstream &f);
public:
    GameManager(PlayerFactory &player_factory, TankAlgorithmFactory &algorithmFactory);
    ~GameManager() {}
    void readBoard(string fileMame);
    void run();
};
