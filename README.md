# Tank Game Ex2

# Contributors
- Name: Nir Ben Dov, ID: 211962212
- Name: David Goldstein, ID: 208000547

# Compilation and Running Instructions

## Compilation
To compile the project, use the following command:
```bash
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -o tank_game.exe Main.cpp common/*.cpp game_management/*.cpp -I. -Iconstants
```

## Running the Game
To run the game with a specific input file, use:
```bash
./tank_game.exe input1.txt
```

You can also run with other input files:
```bash
./tank_game.exe input2.txt
./tank_game.exe input3.txt
```

The program will read the input file and execute the tank game simulation according to the specified parameters.
