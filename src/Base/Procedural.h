//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <ion/gfx/image.h>

struct Color;

/// \file
/// This file contains functions used for procedural images.
///
/// \ingroup Base

/// Generates the grid image used for the Stage. The radius of the Stage is
/// supplied because it depends on the build volume size. The colors to use for
/// the X and Y axis lines are also supplied.
///
/// \ingroup Base
ion::gfx::ImagePtr GenerateGridImage(float radius,
                                     const Color &x_color,
                                     const Color &y_color);

/// Generates the color image used for the ColorTool ring.
///
/// \ingroup Base
ion::gfx::ImagePtr GenerateColorRingImage();
