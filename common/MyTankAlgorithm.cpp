#include "MyTankAlgorithm.h"

ActionRequest MyTankAlgorithm::getAction() {
    return ActionRequest::DoNothing;
}

void MyTankAlgorithm::updateBattleInfo(BattleInfo& /*info*/) {
    // Do nothing for now
} 