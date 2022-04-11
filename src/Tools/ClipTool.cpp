#include "Tools/ClipTool.h"

#include "Managers/CommandManager.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/SphereWidget.h"

// ----------------------------------------------------------------------------
// ClipTool::Parts_ struct.
// ----------------------------------------------------------------------------

/// This struct stores all of the parts the ClipTool needs to operate.
struct ClipTool::Parts_ {
    /// SphereWidget to rotate the clipping plane.
    SphereWidgetPtr     rotator;

    /// Slider1DWidget for translating the clipping plane.
    Slider1DWidgetPtr   slider;

    /// PushButtonWidget for applying the clip plane.
    PushButtonWidgetPtr button;
};

// ----------------------------------------------------------------------------
// ClipTool functions.
// ----------------------------------------------------------------------------

ClipTool::ClipTool() {
}

void ClipTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate())
        FindParts_();
}

void ClipTool::UpdateGripInfo(GripInfo &info) {
    // XXXX
}

void ClipTool::Attach() {
    // XXXX
}

void ClipTool::Detach() {
    // Nothing to do here.
}

void ClipTool::FindParts_() {
    ASSERT(! parts_);
    parts_.reset(new Parts_);
    // XXXX
}
