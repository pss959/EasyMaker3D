#pragma once

#ifdef DEBUG  // Don't include this in release builds.

#include "SG/Hit.h"
#include "SceneContext.h"

namespace Debug {

void ShowHit(const SceneContext &context, const SG::Hit &hit);

}  // namespace Debug

#endif
