#pragma once
#include "TankAlgorithmFactory.h"

class MyTankAlgorithmFactory : public TankAlgorithmFactory {
public:
    MyTankAlgorithmFactory() = default;
    ~MyTankAlgorithmFactory() override = default;
    
    std::unique_ptr<TankAlgorithm> create(int player_index, int tank_index) const override;
}; 