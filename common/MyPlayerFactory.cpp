#include "MyPlayerFactory.h"
#include "Player1.h"
#include "Player2.h"
#include <stdexcept>
#include <string>

std::unique_ptr<Player> MyPlayerFactory::create(int player_index,
                                              size_t x, size_t y,
                                              size_t max_steps, size_t num_shells) const {
    if (player_index == 1) {
        return std::make_unique<Player1>(player_index, x, y, max_steps, num_shells);
    } else if (player_index == 2) {
        return std::make_unique<Player2>(player_index, x, y, max_steps, num_shells);
    }
    throw std::runtime_error("Invalid player index: " + std::to_string(player_index));
} 