#include "GameManager.h"

using namespace std;
using namespace BoardConstants;

GameManager::GameManager(PlayerFactory &player_factory, TankAlgorithmFactory &algorithmFactory)
{
}

GameManager::~GameManager()
{
}

// Function to log errors to board_errors.txt
void logError(const string& errorMessage) {
    ofstream errorFile("board_errors.txt", ios::app);
    if (errorFile.is_open()) {
        errorFile << errorMessage << endl;
        errorFile.close();
    }
}

size_t extractVal(ifstream &f, string param) {
    string line;
    if (!getline(f, line)) {
        logError("Error: " + param + " not specified");
        throw runtime_error(param + " not specified");
    }
    size_t equal_sign = line.find("=");
    if (equal_sign == string::npos) {
        logError("Error: Invalid format: " + line);
        throw runtime_error("Invalid format: " + line);
    }
    string val = line.substr(equal_sign + 1);
    size_t space = val.find_last_of(' ');

    try {
        size_t returnVal;
        if (space == string::npos) 
            returnVal = stoi(val);
        else
            returnVal = stoi(val.substr(space + 1));
        cout << param << " = " << returnVal << endl;
        return returnVal;
    }
    catch (const invalid_argument &e) {
        logError("Error: Invalid format: Expecting " + param + " to be unsigned int" + line);
        throw runtime_error("Invalid format: Expecting " + param + " to be unsigned int" + line);
    }
}

void GameManager::buildBoard(ifstream &f) {
    int line_number = 6;
    string s;

    cout << "Reading board contents..." << endl;
    while(getline(f,s)) {
        cout << "Reading line " << line_number << ": " << s << endl;

        if (board.size() >= gameData.rows) {
            cout << "Reached maximum board height, stopping." << endl;
            logError("Warning: File has more rows than specified height. Extra rows will be ignored.");
            break;
        }

        vector<char> row;
        for (size_t i = 0; i < gameData.columns; i++) {
            char c;
            if (i < s.length()) {
                c = s[i];
                // Validate characters
                if (c != WALL && c != DAMAGED_WALL && c != MINE && c != PLAYER1_TANK && c != PLAYER2_TANK && c != EMPTY_SPACE) {
                    logError("Warning: Invalid character '" + string(1, c) + "' at line " + to_string(line_number) + ", position " + to_string(i) + ". Replacing with empty space.");
                    c = EMPTY_SPACE;
                }
            } else {
                c = EMPTY_SPACE;
                logError("Warning: Line " + to_string(line_number) + " is shorter than specified width. Adding empty spaces.");
            }
            row.push_back(c);
        }

        board.push_back(row);
        ++line_number;
    }

    while (board.size() < gameData.rows) {
        logError("Warning: File has fewer rows than specified height. Adding empty rows.");
        vector<char> emptyRow(gameData.columns, EMPTY_SPACE);
        board.push_back(emptyRow);
    }

    std::cout << "Finished constructing board" << std::endl;
}

void GameManager::readBoard(string fileName) {
    ifstream f(fileName);
    if (!f.is_open()) {
        logError("Error: Could not open file: " + fileName);
        throw runtime_error("Could not open file: " + fileName);
    }

    if (!getline(f, gameData.mapName)) {
        logError("Error: File is empty: " + fileName);
        throw runtime_error("File is empty: " + fileName);
    }
    cout << "BoardName = " << gameData.mapName << endl;
    gameData.maxStep = extractVal(f, "MaxSteps");
    gameData.numShells = extractVal(f, "NumShells");
    gameData.rows = extractVal(f, "Rows");
    gameData.columns = extractVal(f, "Columns");
    buildBoard(f);
}
