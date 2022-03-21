#pragma once

#include <ion/gfx/image.h>

/// \file
/// This file contains functions used for procedural images.

/// Generates the grid image used for the Stage. The radius of the Stage is
/// supplied because it depends on the build volume size.
ion::gfx::ImagePtr GenerateGridImage(float radius);

/// Generates the color image used for the ColorTool ring.
ion::gfx::ImagePtr GenerateColorRingImage();
