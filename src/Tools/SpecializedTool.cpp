#include "Tools/SpecializedTool.h"

#include "Selection.h"

bool SpecializedTool::CanAttach(const Selection &sel) const {
    // A specialized tool can be used only if it can be attached to all
    // selected Models.
    for (auto &sel_path: sel.GetPaths())
        if (! CanAttachToModel(*sel_path.GetModel()))
            return false;
    return true;
}

void SpecializedTool::Finish() {
    if (completion_func_)
        completion_func_(GetSelection());
}
