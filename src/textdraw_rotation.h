#pragma once

#include <cstdint>

namespace textdraw_rotation {

bool Initialize();
void Process();
void Shutdown();

bool Set(uint16_t textDrawIndex, float angle, float pivotX, float pivotY, float spinSpeed);
void Reset(uint16_t textDrawIndex);
void ClearAllStates();

}
