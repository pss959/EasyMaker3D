#pragma once

#include "Math/Types.h"

/// \file
/// This file contains functions to aid with animations.
/// \ingroup Math

/// Returns a modified t value (in [0,1]) that applies damping to speed up at
/// the start and slow down at the end of an animation.
float Dampen(float t);
