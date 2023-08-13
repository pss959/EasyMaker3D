#include "Tools/MirrorTool.h"

#include "Commands/ChangeMirrorCommand.h"
#include "Models/MirroredModel.h"
#include "Util/Assert.h"

MirrorTool::MirrorTool() {
}

bool MirrorTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<MirroredModel>(sel);
}

Plane MirrorTool::GetObjectPlaneFromModel() const {
    auto mm = std::dynamic_pointer_cast<MirroredModel>(GetModelAttachedTo());
    ASSERT(mm);
    return Plane(0, mm->GetPlaneNormal());
}

Range1f MirrorTool::GetTranslationRange() const {
    return Range1f(-100, 100);
}

ChangePlaneCommandPtr MirrorTool::CreateChangePlaneCommand() const {
    return Command::CreateCommand<ChangeMirrorCommand>();
}

Point3f MirrorTool::GetTranslationFeedbackBasePoint() const {
    // Always use the center of the operand Model.
    auto mm = std::dynamic_pointer_cast<MirroredModel>(GetModelAttachedTo());
    ASSERT(mm);
    return Point3f(mm->GetOperandModel()->GetTranslation());
}
