#pragma once
#include <memory>
#include <cstddef>
#include "Player.h"

using namespace std;

class PlayerFactory
{
private:
    /* data */
public:
    PlayerFactory(/* args */);
    virtual ~PlayerFactory() {}
    virtual unique_ptr<Player> create(int player_index,
                                    size_t x, size_t y,
                                    size_t max_steps, size_t num_shells) const = 0;
};
