#pragma once
#include "ActionRequest.h"

struct RoundInfo {
    bool isAlive;
    ActionRequest action;
    bool wasActionIgnored;
    bool wasKilled;
}; 