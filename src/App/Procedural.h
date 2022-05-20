#pragma once

#include <ion/gfx/image.h>

/// \file
/// This file contains functions used for procedural images.
///
/// \ingroup App

/// Generates the grid image used for the Stage. The radius of the Stage is
/// supplied because it depends on the build volume size.
///
/// \ingroup App
ion::gfx::ImagePtr GenerateGridImage(float radius);

/// Generates the color image used for the ColorTool ring.
///
/// \ingroup App
ion::gfx::ImagePtr GenerateColorRingImage();
