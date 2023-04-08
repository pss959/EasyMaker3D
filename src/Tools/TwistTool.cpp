#include "Tools/TwistTool.h"

#include "Models/TwistedModel.h"
#include "Util/Assert.h"

bool TwistTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TwistedModel>(sel);
}

void TwistTool::Attach() {
    ASSERT(Util::IsA<TwistedModel>(GetModelAttachedTo()));
    // XXXX DO SOMETHING.
}

void TwistTool::Detach() {
    // Nothing to do here.
}
