#include "GameManager.h"
#include "../common/GameSatelliteView.h"
#include <algorithm>

using namespace std;
using namespace BoardConstants;


GameManager::GameManager(PlayerFactory &player_factory, TankAlgorithmFactory &algorithmFactory)
    : playerFactory(player_factory), algorithmFactory(algorithmFactory), creationOrderCounter(0),
      allTanksOutOfShells(false), roundsSinceNoShells(0)
{
}

void GameManager::readBoard(string fileName) {
    inputFileName = fileName;  // Store the input filename
    gameData = BoardReader::readBoard(fileName);
    std::cout << "Game data: " << gameData.rows << " " << gameData.columns << std::endl;
}

void GameManager::setOutputFile() {
    // Create output filename based on input filename
    string outputFileName = "output_" + inputFileName;
    outputWriter = make_unique<OutputWriter>(outputFileName);
}

bool GameManager::checkAllTanksOutOfShells() {
    // Check player 1 tanks
    for (const auto& tank : player1Tanks) {
        if (tank.getIsAlive() && tank.getNumShells() > 0) {
            return false;
        }
    }
    // Check player 2 tanks
    for (const auto& tank : player2Tanks) {
        if (tank.getIsAlive() && tank.getNumShells() > 0) {
            return false;
        }
    }
    return true;
}

bool GameManager::checkImmediateGameEnd() {
    std::cout << "Checking immediate game end conditions:" << std::endl;
    std::cout << "Player 1 tanks: " << gameData.player1TankCount << std::endl;
    std::cout << "Player 2 tanks: " << gameData.player2TankCount << std::endl;
    
    // Check if all tanks are out of shells
    if (!allTanksOutOfShells && checkAllTanksOutOfShells()) {
        allTanksOutOfShells = true;
        roundsSinceNoShells = 0;
        std::cout << "All tanks have run out of shells" << std::endl;
    }
    
    // If all tanks are out of shells, increment the counter
    if (allTanksOutOfShells) {
        roundsSinceNoShells++;
        std::cout << "Rounds since all tanks ran out of shells: " << roundsSinceNoShells << std::endl;
        
        // Check if 40 rounds have passed since all tanks ran out of shells
        if (roundsSinceNoShells >= OutputWriter::ZERO_SHELLS_STEPS) {
            std::cout << "Game end: 40 rounds have passed since all tanks ran out of shells" << std::endl;
            outputWriter->writeZeroShellsTie();
            return true;
        }
    }
    
    if (gameData.player1TankCount == 0 && gameData.player2TankCount == 0) {
        std::cout << "Game end: Both players have no tanks remaining - Tie" << std::endl;
        outputWriter->writeGameEnd(0, 0); // Tie
        return true;
    } else if (gameData.player1TankCount == 0) {
        std::cout << "Game end: Player 1 has no tanks remaining - Player 2 wins" << std::endl;
        outputWriter->writeGameEnd(2, gameData.player2TankCount);
        return true;
    } else if (gameData.player2TankCount == 0) {
        std::cout << "Game end: Player 2 has no tanks remaining - Player 1 wins" << std::endl;
        outputWriter->writeGameEnd(1, gameData.player1TankCount);
        return true;
    }
    
    std::cout << "No immediate game end conditions met" << std::endl;
    return false;
}

vector<TankPosition> GameManager::collectTankPositions() {
    vector<TankPosition> tankPositions;
    
    // Collect player 1 tank positions
    int tankIndex1 = 0;
    for (size_t y = 0; y < gameData.rows; y++) {
        for (size_t x = 0; x < gameData.columns; x++) {
            if (gameData.board[y][x] == PLAYER1_TANK) {
                tankPositions.push_back({x, y, 1, tankIndex1++});
            }
        }
    }
    
    // Collect player 2 tank positions
    int tankIndex2 = 0;
    for (size_t y = 0; y < gameData.rows; y++) {
        for (size_t x = 0; x < gameData.columns; x++) {
            if (gameData.board[y][x] == PLAYER2_TANK) {
                tankPositions.push_back({x, y, 2, tankIndex2++});
            }
        }
    }
    
    return tankPositions;
}

void GameManager::sortTankPositions(vector<TankPosition>& positions) {
    sort(positions.begin(), positions.end(), 
        [](const TankPosition& a, const TankPosition& b) {
            if (a.y != b.y) return a.y < b.y;
            return a.x < b.x;
        });
}

void GameManager::createTanksFromPositions(const vector<TankPosition>& positions) {
    for (const auto& pos : positions) {
        int dx = (pos.playerId == 1) ? -1 : 1;  // Player 1 faces left (-1,0), Player 2 faces right (1,0)
        int dy = 0;  // Both players start with horizontal direction
        
        auto algorithm = algorithmFactory.create(pos.playerId, pos.tankIndex);
        if (pos.playerId == 1) {
            player1Tanks.emplace_back(pos.x, pos.y, dx, dy, std::move(algorithm), 
                                    gameData.columns, gameData.rows, pos.playerId, creationOrderCounter++, gameData.numShells);
        } else {
            player2Tanks.emplace_back(pos.x, pos.y, dx, dy, std::move(algorithm), 
                                    gameData.columns, gameData.rows, pos.playerId, creationOrderCounter++, gameData.numShells);
        }
    }
}

void GameManager::initializePlayersAndTanks() {
    // Clear any existing tanks
    player1Tanks.clear();
    player2Tanks.clear();
    
    // Create players with board dimensions
    playerOne = playerFactory.create(1, gameData.columns, gameData.rows, gameData.maxStep, gameData.numShells);
    playerTwo = playerFactory.create(2, gameData.columns, gameData.rows, gameData.maxStep, gameData.numShells);
    
    // Reset creation order counter
    creationOrderCounter = 0;
    
    // Collect and sort tank positions
    auto tankPositions = collectTankPositions();
    sortTankPositions(tankPositions);
    
    // Create tanks in sorted order
    createTanksFromPositions(tankPositions);
}

void GameManager::detectShellCrossings(vector<bool>& shellsToRemove, map<pair<size_t, size_t>, vector<size_t>>& nextPositions) {
    // First pass: collect all potential moves and detect crossings
    for (size_t i = 0; i < activeShells.size(); i++) {
        auto nextPos = activeShells[i].getPotentialMove();
        auto currentPos = make_pair(activeShells[i].getX(), activeShells[i].getY());
        bool isCrossing = false;
        
        // Check if another shell is moving to this position
        auto it = nextPositions.find(nextPos);
        if (it != nextPositions.end()) {
            // Another shell is moving here - check if they're crossing
            for (size_t otherShell : it->second) {
                auto otherNextPos = make_pair(activeShells[otherShell].getX(), activeShells[otherShell].getY());
                auto otherCurrentPos = make_pair(activeShells[otherShell].getX(), activeShells[otherShell].getY());
                
                // Check both directions of crossing
                isCrossing = (otherNextPos == currentPos && nextPos == otherCurrentPos);
                
                if (isCrossing) {
                    // Shells are crossing - mark both for removal
                    shellsToRemove[i] = true;
                    shellsToRemove[otherShell] = true;
                    break;
                }
            }
        }
        
        // Only record this shell's move if it's not crossing
        if (!isCrossing) {
            nextPositions[nextPos].push_back(i);
        }
    }
}

void GameManager::removeMarkedShells(const vector<bool>& shellsToRemove) {
    // Remove shells that are crossing
    for (int i = activeShells.size() - 1; i >= 0; i--) {
        if (shellsToRemove[i]) {
            activeShells.erase(activeShells.begin() + i);
        }
    }
}

void GameManager::findAndKillTank(size_t x, size_t y) {
    // Check player 1 tanks
    for (auto& tank : player1Tanks) {
        if (tank.getX() == x && tank.getY() == y && tank.getIsAlive()) {
            tank.killTank();
            gameData.player1TankCount--;
            return;
        }
    }
    // Check player 2 tanks
    for (auto& tank : player2Tanks) {
        if (tank.getX() == x && tank.getY() == y && tank.getIsAlive()) {
            tank.killTank();
            gameData.player2TankCount--;
            return;
        }
    }
}

void GameManager::handleTankCollision(const pair<size_t, size_t>& pos, const vector<size_t>& shellIndices) {
    // Kill the tank
    findAndKillTank(pos.first, pos.second);
    
    // Mark position as empty
    gameData.board[pos.second][pos.first] = EMPTY_SPACE;
}

void GameManager::handleWallCollision(const pair<size_t, size_t>& pos, const vector<size_t>& shellIndices) {
    // we their is only one shell, so we can just destroy the wall
    gameData.board[pos.second][pos.first] = DAMAGED_WALL;
}

void GameManager::handleDamagedWallCollision(const pair<size_t, size_t>& pos, const vector<size_t>& shellIndices) {
    // Any number of shells destroys a damaged wall
    gameData.board[pos.second][pos.first] = EMPTY_SPACE;
}

void GameManager::handleMineCollision(const pair<size_t, size_t>& pos) {
    gameData.board[pos.second][pos.first] = MINE_SHELL_COLLISION;
}

void GameManager::handleMultipleShellCollision(const pair<size_t, size_t>& pos) {
    // Check what's at this position and handle accordingly
    char currentCell = gameData.board[pos.second][pos.first];
    switch (currentCell) {
        case PLAYER1_TANK:
        case PLAYER2_TANK:
            // Kill the tank
            findAndKillTank(pos.first, pos.second);
            break;
        case WALL:
        case DAMAGED_WALL:
            // Walls are destroyed by multiple shells
            break;
        case MINE:
            // Mine is destroyed by multiple shells
            break;
        // For other cases (empty, shell, collision states) we just clear the position
    }
    
    // Multiple shells destroy everything
    gameData.board[pos.second][pos.first] = EMPTY_SPACE;
}

void GameManager::handleShellPositions(const map<pair<size_t, size_t>, vector<size_t>>& nextPositions) {
    for (const auto& [pos, shellIndices] : nextPositions) {
        if (shellIndices.size() > 1) {
            // Multiple shells in same position - destroy everything
            handleMultipleShellCollision(pos);
            continue;
        }

        // Single shell - check what's in the target position
        char nextCell = gameData.board[pos.second][pos.first];
        switch (nextCell) {
            case PLAYER1_TANK:
            case PLAYER2_TANK:
                handleTankCollision(pos, shellIndices);
                break;
            case WALL:
                handleWallCollision(pos, shellIndices);
                break;
            case DAMAGED_WALL:
                handleDamagedWallCollision(pos, shellIndices);
                break;
            case MINE:
                handleMineCollision(pos);
                break;
            case EMPTY_SPACE:
                gameData.board[pos.second][pos.first] = SHELL;
                break;
        }
    }
}

void GameManager::moveShells() {
    vector<bool> shellsToRemove(activeShells.size(), false);
    map<pair<size_t, size_t>, vector<size_t>> nextPositions;  // Map of position to shell indices
    
    // Detect crossings and collect next positions
    detectShellCrossings(shellsToRemove, nextPositions);
    
    // First clear all current shell positions, but only if there isn't a tank there
    for (const auto& shell : activeShells) {
        char currentCell = gameData.board[shell.getY()][shell.getX()];
        if (currentCell == SHELL) {  // Only clear if it's a shell (not a tank)
            gameData.board[shell.getY()][shell.getX()] = EMPTY_SPACE;
        } else if (currentCell == MINE_SHELL_COLLISION) {  // If it was a mine-shell collision, change back to mine
            gameData.board[shell.getY()][shell.getX()] = MINE;
        }
    }
    
    // Remove shells that are crossing
    removeMarkedShells(shellsToRemove);
    
    // Handle shell positions and collisions
    handleShellPositions(nextPositions);
    
    // Move remaining shells
    for (auto& shell : activeShells) {
        shell.move();
    }
}

void GameManager::checkCollisions() {
    // TODO: Implement collision detection
    // Check for collisions between:
    // 1. Shells and walls
    // 2. Shells and tanks
    // 3. Shells and other shells
    // 4. Tanks and walls
    // 5. Tanks and other tanks
}

void GameManager::updateTanks() {
    std::cout << "\nUpdating Player 1 tanks..." << std::endl;
    // Process player 1 tanks
    updateTankVector(player1Tanks);
    
    std::cout << "Updating Player 2 tanks..." << std::endl;
    // Process player 2 tanks
    updateTankVector(player2Tanks);
    
    std::cout << "Checking for tank swapping..." << std::endl;
    // Check for tank swapping after all moves are made
    checkTankSwapping();
}

void GameManager::updateTankVector(vector<TankInfo>& tanks) {
    for (size_t i = 0; i < tanks.size(); i++) {
        auto& tank = tanks[i];
        
        if (!tank.getIsAlive() && !tank.getRoundWasKilled()) {
            std::cout << "Tank " << i << " (Player " << tank.getPlayerId() << ") is dead, skipping..." << std::endl;
            continue;
        }
        
        std::cout << "Processing Tank " << i << " (Player " << tank.getPlayerId() << ") at position (" 
                  << tank.getX() << "," << tank.getY() << ")" << std::endl;
        
        // Get action from tank's algorithm
        auto action = tank.getAlgorithm()->getAction();
        std::cout << "Tank " << i << " chose action: " << static_cast<int>(action) << std::endl;
        
        // Store the action in tank's round info
        tank.setRoundAction(action);
        tank.setRoundIsAlive(tank.getIsAlive());
        
        // Process the action
        processTankAction(tank, action);
        
        if (tank.getRoundWasActionIgnored()) {
            std::cout << "Tank " << i << "'s action was ignored" << std::endl;
        }
        if (tank.getRoundWasKilled()) {
            std::cout << "Tank " << i << " was killed" << std::endl;
        }
    }
}

map<pair<size_t, size_t>, TankInfo*> GameManager::createTankPositionMap() {
    map<pair<size_t, size_t>, TankInfo*> currentPositions;
    
    // Add all alive tanks to the map
    for (auto& tank : player1Tanks) {
        if (tank.getIsAlive()) {
            currentPositions[{tank.getX(), tank.getY()}] = &tank;
        }
    }
    for (auto& tank : player2Tanks) {
        if (tank.getIsAlive()) {
            currentPositions[{tank.getX(), tank.getY()}] = &tank;
        }
    }
    
    return currentPositions;
}

bool GameManager::tanksSwappedPlaces(TankInfo* tank1, TankInfo* tank2) {
    if (!tank1 || !tank2) {
        std::cout << "Invalid tank pointer in swap check" << std::endl;
        return false;
    }
    if (!tank1->getIsAlive() || !tank2->getIsAlive()) {
        std::cout << "One or both tanks are dead in swap check" << std::endl;
        return false;
    }
    
    // Get the tanks' previous positions
    auto prevPos1 = tank1->getPreviousPosition();
    auto prevPos2 = tank2->getPreviousPosition();
    
    if (!prevPos1 || !prevPos2) {
        std::cout << "One or both tanks have no previous position" << std::endl;
        return false;
    }
    
    // Check if tanks swapped places
    bool swapped = (prevPos1->first == tank2->getX() && prevPos1->second == tank2->getY() &&
                   prevPos2->first == tank1->getX() && prevPos2->second == tank1->getY());
    
    if (swapped) {
        std::cout << "Confirmed swap: Tank " << tank1->getCreationOrder() << " moved from ("
                  << prevPos1->first << "," << prevPos1->second << ") to (" 
                  << tank1->getX() << "," << tank1->getY() << ")" << std::endl;
        std::cout << "              Tank " << tank2->getCreationOrder() << " moved from ("
                  << prevPos2->first << "," << prevPos2->second << ") to (" 
                  << tank2->getX() << "," << tank2->getY() << ")" << std::endl;
    }
    
    return swapped;
}

void GameManager::handleTankSwap(TankInfo* tank1, TankInfo* tank2) {
    std::cout << "\nHandling tank swap collision:" << std::endl;
    std::cout << "Tank " << tank1->getCreationOrder() << " (Player " << tank1->getPlayerId() 
              << ") at (" << tank1->getX() << "," << tank1->getY() << ")" << std::endl;
    std::cout << "Tank " << tank2->getCreationOrder() << " (Player " << tank2->getPlayerId() 
              << ") at (" << tank2->getX() << "," << tank2->getY() << ")" << std::endl;
    
    // Kill both tanks
    tank1->killTank();
    tank1->setRoundWasKilled(true);
    gameData.board[tank1->getY()][tank1->getX()] = EMPTY_SPACE;
    
    tank2->killTank();
    tank2->setRoundWasKilled(true);
    gameData.board[tank2->getY()][tank2->getX()] = EMPTY_SPACE;
    
    // Update tank counts
    if (tank1->getPlayerId() == 1) {
        gameData.player1TankCount--;
    } else {
        gameData.player2TankCount--;
    }
    if (tank2->getPlayerId() == 1) {
        gameData.player1TankCount--;
    } else {
        gameData.player2TankCount--;
    }
    
    std::cout << "Both tanks destroyed. Remaining tanks - Player 1: " 
              << gameData.player1TankCount << ", Player 2: " << gameData.player2TankCount << std::endl;
}

void GameManager::checkTankSwapping() {
    std::cout << "\nChecking for tank swapping..." << std::endl;
    // Create a map of current positions to tank pointers
    auto currentPositions = createTankPositionMap();
    
    // Check for tanks that swapped places
    for (auto& [pos, tank] : currentPositions) {
        if (!tank->getIsAlive()) {
            std::cout << "Skipping dead tank at position (" << pos.first << "," << pos.second << ")" << std::endl;
            continue;  // Skip if tank was already killed
        }
        
        // Get the tank's previous position
        auto prevPos = tank->getPreviousPosition();
        if (!prevPos) {
            std::cout << "Tank " << tank->getCreationOrder() << " (Player " << tank->getPlayerId() 
                      << ") has no previous position, skipping" << std::endl;
            continue;  // Skip if no previous position (tank didn't move)
        }
        
        std::cout << "Checking tank " << tank->getCreationOrder() << " (Player " << tank->getPlayerId() 
                  << ") at (" << pos.first << "," << pos.second << ") with previous position ("
                  << prevPos->first << "," << prevPos->second << ")" << std::endl;
        
        // Check if there's another tank at the previous position
        auto otherTankIt = currentPositions.find(*prevPos);
        if (otherTankIt != currentPositions.end()) {
            TankInfo* otherTank = otherTankIt->second;
            
            std::cout << "Found another tank " << otherTank->getCreationOrder() << " (Player " 
                      << otherTank->getPlayerId() << ") at previous position" << std::endl;
            
            // Check if tanks swapped places
            if (tanksSwappedPlaces(tank, otherTank)) {
                std::cout << "Tanks " << tank->getCreationOrder() << " and " << otherTank->getCreationOrder() 
                          << " swapped places - handling collision" << std::endl;
                handleTankSwap(tank, otherTank);
            }
        }
    }
}

bool GameManager::isValidTankAction(const TankInfo& tank, ActionRequest action) const {
    // If tank is dead, no actions are valid
    if (!tank.getIsAlive()) {
        return false;
    }

    // If tank is in backward movement sequence, only forward move is valid to cancel it
    if (tank.getIsMovingBackward()) {
        return action == ActionRequest::MoveForward;
    }

    // Check specific action validations
    switch (action) {
        case ActionRequest::MoveForward: {
            // Get the potential move position
            auto [nextX, nextY] = tank.getPotentialMove();
            
            // Check if the move is valid (not into a wall or damaged wall)
            char nextCell = gameData.board[nextY][nextX];
            return nextCell != WALL && nextCell != DAMAGED_WALL;
        }
            
        case ActionRequest::Shoot:
            // Can only shoot if cooldown is 0 and tank has shells
            return tank.getShootCooldown() == 0 && tank.getNumShells() > 0;
            
        case ActionRequest::GetBattleInfo:
        case ActionRequest::DoNothing:
        case ActionRequest::RotateLeft90:
        case ActionRequest::RotateRight90:
        case ActionRequest::RotateLeft45:
        case ActionRequest::RotateRight45:
        case ActionRequest::MoveBackward: {
            if (tank.getIsMovingBackward() && tank.getBackwardMoveCounter() == 2) {
            // Get the potential move position
            auto [nextX, nextY] = tank.getPotentialMoveBackwards();
            
            // Check if the move is valid (not into a wall or damaged wall)
            char nextCell = gameData.board[nextY][nextX];
            return nextCell != WALL && nextCell != DAMAGED_WALL;
            }
            return true;
        }
        default:
            return false;
    }
}

void GameManager::processTankAction(TankInfo& tank, ActionRequest action) {
    // If tank is dead, mark action as ignored
    if (!tank.getIsAlive()) {
        std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                  << ") is dead, ignoring action" << std::endl;
        tank.setRoundWasActionIgnored(isValidTankAction(tank, action));
        return;
    }

    // If tank is in backward movement sequence, only allow forward move to cancel
    if (tank.getIsMovingBackward()) {
        if (action == ActionRequest::MoveForward) {
            std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                      << ") cancelling backward movement with forward move" << std::endl;
            tank.cancelBackwardMove();
            tank.setRoundWasActionIgnored(false);
            return;
        }
        // All other actions are ignored during backward movement
        std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                  << ") is in backward movement sequence, ignoring action" << std::endl;
        tank.setRoundWasActionIgnored(true);
        return;
    }
    
    // Reset action ignored flag at the start of processing
    tank.setRoundWasActionIgnored(false);
    
    switch (action) {
        case ActionRequest::MoveForward: {
            // Get the potential move position
            auto [nextX, nextY] = tank.getPotentialMove();
            
            // Check if the move is valid (not into a wall or damaged wall)
            char nextCell = gameData.board[nextY][nextX];
            
            std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                      << ") attempting to move forward to (" << nextX << "," << nextY << ")" << std::endl;
            
            if (nextCell == WALL || nextCell == DAMAGED_WALL) {
                // Move was invalid - mark as ignored
                std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                          << ") cannot move forward - blocked by " << nextCell << std::endl;
                tank.setRoundWasActionIgnored(true);
                break;
            }

            // Store current position before moving
            size_t prevX = tank.getX();
            size_t prevY = tank.getY();

            // Move tank
            tank.move();
            
            std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                      << ") moved from (" << prevX << "," << prevY << ") to (" 
                      << tank.getX() << "," << tank.getY() << ")" << std::endl;
            
            // Update both current and next positions
            gameData.board[prevY][prevX] = getCurrentCellState(prevX, prevY);
            gameData.board[tank.getY()][tank.getX()] = getNextCellState(nextCell, tank);
            break;
        }
            
        case ActionRequest::MoveBackward: {
            // Start backward movement sequence
            tank.startBackwardMove();
            tank.setRoundWasActionIgnored(false);

            std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                      << ") starting backward movement sequence (step " << tank.getBackwardMoveCounter() << ")" << std::endl;

            // If this is the third step of backward movement, perform the move
            if (tank.getBackwardMoveCounter() == 2) {
                // Get the potential backward move position
                auto [nextX, nextY] = tank.getPotentialMoveBackwards();
                
                // Check if the move is valid
                char nextCell = gameData.board[nextY][nextX];
                
                std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                          << ") attempting to complete backward move to (" << nextX << "," << nextY << ")" << std::endl;
                
                if (nextCell == WALL || nextCell == DAMAGED_WALL) {
                    // Move was invalid - mark as ignored
                    std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                              << ") cannot complete backward move - blocked by " << nextCell << std::endl;
                    tank.setRoundWasActionIgnored(true);
                    break;
                }

                // Store current position before moving
                size_t prevX = tank.getX();
                size_t prevY = tank.getY();

                // Move tank backward
                tank.moveBackwards();
                
                std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                          << ") completed backward move from (" << prevX << "," << prevY << ") to (" 
                          << tank.getX() << "," << tank.getY() << ")" << std::endl;
                
                // Update both current and next positions
                gameData.board[prevY][prevX] = getCurrentCellState(prevX, prevY);
                gameData.board[tank.getY()][tank.getX()] = getNextCellState(nextCell, tank);
            }
            break;
        }
            
        case ActionRequest::RotateLeft90:
        case ActionRequest::RotateRight90:
        case ActionRequest::RotateLeft45:
        case ActionRequest::RotateRight45: {
            std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                      << ") rotating " << (action == ActionRequest::RotateLeft90 || action == ActionRequest::RotateLeft45 ? "left" : "right") 
                      << " " << (action == ActionRequest::RotateLeft90 || action == ActionRequest::RotateRight90 ? "90" : "45") 
                      << " degrees" << std::endl;
            tank.rotate(action);
            break;
        }
            
        case ActionRequest::Shoot:
            if (tank.getShootCooldown() == 0 && tank.getNumShells() > 0) {
                std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                          << ") shooting from position (" << tank.getX() << "," << tank.getY() << ")" << std::endl;
                addShell(tank);
                tank.startShootCooldown();
                tank.setNumShells(tank.getNumShells() - 1);  // Decrease number of shells
            } else {
                // Shoot was invalid due to cooldown or no shells - mark as ignored
                std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                          << ") cannot shoot - cooldown: " << tank.getShootCooldown() 
                          << " turns remaining, shells: " << tank.getNumShells() << std::endl;
                tank.setRoundWasActionIgnored(true);
            }
            break;
            
        case ActionRequest::GetBattleInfo: {
            std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                      << ") requesting battle info" << std::endl;
            // Create a GameSatelliteView with the board state from the start of the round
            GameSatelliteView satelliteView(roundStartBoard, gameData.rows, gameData.columns, tank.getX(), tank.getY());
            
            // Get the appropriate player based on tank's player ID
            Player* player = (tank.getPlayerId() == 1) ? playerOne.get() : playerTwo.get();
            
            // Update the tank's algorithm with battle info
            player->updateTankWithBattleInfo(*tank.getAlgorithm(), satelliteView);
            break;
        }
            
        case ActionRequest::DoNothing:
            std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                      << ") doing nothing" << std::endl;
            break;
            
        default:
            std::cout << "Tank " << tank.getCreationOrder() << " (Player " << tank.getPlayerId() 
                      << ") received unknown action: " << static_cast<int>(action) << std::endl;
            break;
    }
}

void GameManager::addShell(const TankInfo& tank) {
    // Create a new shell at the tank's position with the tank's direction
    Shell shell(tank.getX(), tank.getY(), tank.getDirection()[0], tank.getDirection()[1], gameData.columns, gameData.rows);
    activeShells.push_back(shell);
}

char GameManager::getNextCellState(char currentCell, const TankInfo& tank) {
    switch (currentCell) {
        case MINE:
            return TANK_MINE_COLLISION;
        case PLAYER1_TANK:
        case PLAYER2_TANK:
            return TANK_TANK_COLLISION;
        case SHELL:
            return TANK_SHELL_COLLISION;
        case EMPTY_SPACE:
            return (tank.getPlayerId() == 1) ? PLAYER1_TANK : PLAYER2_TANK;
        default:
            // For any other collision state, keep it as is
            return currentCell;
    }
}

char GameManager::getCurrentCellState(size_t x, size_t y) {
    char currentCell = gameData.board[y][x];
    switch (currentCell) {
        case PLAYER1_TANK:
        case PLAYER2_TANK:
            return EMPTY_SPACE;
        case TANK_TANK_COLLISION: {
            // Count how many tanks are at this position
            int tankCount = 0;
            for (const auto& t : player1Tanks) {
                if (t.getIsAlive() && t.getX() == x && t.getY() == y) {
                    tankCount++;
                }
            }
            for (const auto& t : player2Tanks) {
                if (t.getIsAlive() && t.getX() == x && t.getY() == y) {
                    tankCount++;
                }
            }
            
            // If more than one tank, keep collision state, otherwise set to remaining tank
            if (tankCount > 1) {
                return TANK_TANK_COLLISION;
            } else {
                // Find the remaining tank and return its character
                for (const auto& t : player1Tanks) {
                    if (t.getIsAlive() && t.getX() == x && t.getY() == y) {
                        return PLAYER1_TANK;
                    }
                }
                for (const auto& t : player2Tanks) {
                    if (t.getIsAlive() && t.getX() == x && t.getY() == y) {
                        return PLAYER2_TANK;
                    }
                }
            }
            return EMPTY_SPACE;  // Should never reach here
        }
        default:
            return currentCell;  // Keep other states as is
    }
}

void GameManager::logRound() {
    std::cout << "\nLogging round information..." << std::endl;
    
    // Get all tanks in a single vector using references
    vector<reference_wrapper<TankInfo>> allTanks;
    for (auto& tank : player1Tanks) {
        allTanks.push_back(ref(tank));  // Log all tanks, including dead ones
    }
    for (auto& tank : player2Tanks) {
        allTanks.push_back(ref(tank));  // Log all tanks, including dead ones
    }
    
    // Log all tanks
    for (const auto& tank : allTanks) {
        try {
            std::cout << "Tank " << tank.get().getCreationOrder() 
                      << " (P" << tank.get().getPlayerId() << "): "
                      << (tank.get().getRoundIsAlive() ? "Alive" : "Dead") << " | "
                      << "Action: " << static_cast<int>(tank.get().getRoundAction())
                      << (tank.get().getRoundWasActionIgnored() ? " (Ignored)" : "")
                      << (tank.get().getRoundWasKilled() ? " (Killed)" : "") << std::endl;
            
            RoundInfo info;
            info.isAlive = tank.get().getRoundIsAlive();
            info.action = tank.get().getRoundAction();
            info.wasActionIgnored = tank.get().getRoundWasActionIgnored();
            info.wasKilled = tank.get().getRoundWasKilled();
            outputWriter->addRoundForTank(tank.get().getCreationOrder(), info);
        } catch (const std::exception& e) {
            std::cerr << "Error logging tank info: " << e.what() << std::endl;
        }
    }
}

void GameManager::printBoard() {
    std::cout << "\nCurrent Board State:" << std::endl;
    for (size_t y = 0; y < gameData.rows; y++) {
        for (size_t x = 0; x < gameData.columns; x++) {
            std::cout << gameData.board[y][x];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void GameManager::runGameLoop() {
    // Main game loop
    for (size_t step = 0; step < gameData.maxStep; step++) {
        std::cout << "\n==================== Round " << step + 1 << " ====================" << std::endl;
        
        // Save the current board state before any movements
        std::cout << "Saving current board state..." << std::endl;
        roundStartBoard = gameData.board;
        
        // Begin new round for all tanks
        std::cout << "Starting new round for all tanks..." << std::endl;
        for (auto& tank : player1Tanks) {
            tank.beginRound();
        }
        for (auto& tank : player2Tanks) {
            tank.beginRound();
        }
        
        // First shell movement
        std::cout << "First shell movement phase..." << std::endl;
        moveShells();
        
        // Second shell movement
        std::cout << "Second shell movement phase..." << std::endl;
        moveShells();
        
        // Update tanks and check collisions
        std::cout << "Updating tanks and checking collisions..." << std::endl;
        updateTanks();
        
        // Log the round information
        std::cout << "Logging round information..." << std::endl;
        logRound();

        // Write the current round to the output file
        std::cout << "Writing round to output file..." << std::endl;
        outputWriter->writeCurrentRound();

        // Print current board state
        std::cout << "Current board state after round " << step + 1 << ":" << std::endl;
        printBoard();
        
        // Print tank counts
        std::cout << "Player 1 tanks remaining: " << gameData.player1TankCount << std::endl;
        std::cout << "Player 2 tanks remaining: " << gameData.player2TankCount << std::endl;

        // Check if game should end
        std::cout << "Checking for game end conditions..." << std::endl;
        if (checkImmediateGameEnd()) {
            std::cout << "Game ended after round " << step + 1 << std::endl;
            return;  // Exit immediately after writing the game end message
        }
        
        std::cout << "Round " << step + 1 << " completed successfully" << std::endl;
    }

    // If we reach here, we hit max steps
    std::cout << "Game reached maximum steps (" << gameData.maxStep << ")" << std::endl;
    outputWriter->writeMaxStepsTie(gameData.maxStep, 
        gameData.player1TankCount, 
        gameData.player2TankCount);
}

void GameManager::run() {
    std::cout << "\nStarting game..." << std::endl;
    std::cout << "Initial board state:" << std::endl;
    printBoard();
    setOutputFile();  // Empty string since we use input filename
    
    if (checkImmediateGameEnd()) {
        std::cout << "Game ended immediately due to initial conditions." << std::endl;
        return;
    }
    
    std::cout << "Initializing players and tanks..." << std::endl;
    initializePlayersAndTanks();
    
    std::cout << "Starting game loop..." << std::endl;
    runGameLoop();
    
    std::cout << "Game finished." << std::endl;
}
