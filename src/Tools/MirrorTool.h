#pragma once

#include "Math/Types.h"
#include "Tools/PlaneBasedTool.h"

/// MirrorTool provides interactive clipping of all selected MirroredModel
/// instances. It is derived from PlaneBasedTool, so it uses a PlaneWidget to
/// orient and position the mirroring plane.
///
/// \ingroup Tools
class MirrorTool : public PlaneBasedTool {
  protected:
    MirrorTool();

    virtual bool IsSpecialized() const override { return true; }
    virtual bool CanAttach(const Selection &sel) const override;

    // Required PlaneBasedTool functions:
    virtual Plane GetObjectPlaneFromModel() const override;
    virtual Range1f GetTranslationRange() const override;
    virtual ChangePlaneCommandPtr CreateChangePlaneCommand() const override;
    virtual Point3f GetTranslationFeedbackBasePoint() const override;

    friend class Parser::Registry;
};
