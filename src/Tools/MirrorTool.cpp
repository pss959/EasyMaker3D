#include "Tools/MirrorTool.h"

#include "Commands/ChangeMirrorCommand.h"
#include "Models/MirroredModel.h"
#include "Util/Assert.h"
#include "Util/General.h"

MirrorTool::MirrorTool() {
}

bool MirrorTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<MirroredModel>(sel);
}

Plane MirrorTool::GetObjectPlaneFromModel() const {
    auto mm = Util::CastToDerived<MirroredModel>(GetModelAttachedTo());
    ASSERT(mm);
    return Plane(0, mm->GetPlaneNormal());
}

Range1f MirrorTool::GetTranslationRange() const {
    return Range1f(-100, 100);
}

ChangePlaneCommandPtr MirrorTool::CreateChangePlaneCommand() const {
    return CreateCommand<ChangeMirrorCommand>();
}
