#include "game_management/GameManager.h"
#include "common/MyPlayerFactory.h"
#include "common/MyTankAlgorithmFactory.h"
#include <string>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <game_board_input_file>" << std::endl;
        return 1;
    }

    MyPlayerFactory playerFactory;
    MyTankAlgorithmFactory algorithmFactory;
    GameManager game(playerFactory, algorithmFactory);
    game.readBoard(argv[1]);
    game.run();
    return 0;
}