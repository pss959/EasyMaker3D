//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"

/// \file
/// This file contains functions to aid with animations.
///
/// \ingroup Math

/// Returns a modified t value (in [0,1]) that applies damping to speed up at
/// the start and slow down at the end of an animation.
float Dampen(float t);
