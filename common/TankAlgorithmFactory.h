#pragma once
#include <memory>
#include <cstddef>
#include "Player.h"
#include "TankAlgorithm.h"

using namespace std;

class TankAlgorithmFactory
{
private:
    /* data */
public:
    TankAlgorithmFactory(/* args */);
    virtual ~TankAlgorithmFactory() {}
    virtual unique_ptr<TankAlgorithm> create(int player_index, int tank_index) const = 0;
};
