#pragma once
#include "TankAlgorithm.h"

class MyTankAlgorithm : public TankAlgorithm {
public:
    MyTankAlgorithm() = default;
    ~MyTankAlgorithm() override = default;
    
    ActionRequest getAction() override;
    void updateBattleInfo(BattleInfo& info) override;
}; 