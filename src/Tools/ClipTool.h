//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "Tools/PlaneBasedTool.h"

/// ClipTool provides interactive clipping of all selected ClippedModel
/// instances. It is derived from PlaneBasedTool, so it uses a PlaneWidget to
/// orient and position the clipping plane.
///
/// Some notes on transformations:
///   - The ClipTool is rotated to align with the primary ClippedModel (even if
///     is_axis_aligned is true).
///   - The PlaneWidget is explicitly sized to surround the ClippedModel. Any
///     scale in the ClippedModel does \em not apply to the PlaneWidget's
///     plane.
///   - The ClipTool is translated so that it is centered on the ClippedModel's
///     operand mesh (i.e., the unclipped mesh) using the center offset in the
///     ClippedModel. This offset has to be dealt with when converting to and
///     from stage coordinates.
///
/// \ingroup Tools
class ClipTool : public PlaneBasedTool {
  protected:
    ClipTool();

    virtual bool CanAttach(const Selection &sel) const override;

    // Required PlaneBasedTool functions:
    virtual Plane GetObjectPlaneFromModel() const override;
    virtual Range1f GetTranslationRange() const override;
    virtual ChangePlaneCommandPtr CreateChangePlaneCommand() const override;
    virtual Point3f GetTranslationFeedbackBasePoint() const override;

    /// Returns the distance range and the min/max points of the scaled operand
    /// mesh along the plane normal.
    void GetRangeAlongPlaneNormal_(Range1f &distance_range,
                                   Range3f &point_range) const;

    friend class Parser::Registry;
};
