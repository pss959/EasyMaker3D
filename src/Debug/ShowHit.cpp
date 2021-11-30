#include "Debug/ShowHit.h"

#include "Util/Assert.h"

namespace Debug {

void ShowHit(const SceneContext &context, const SG::Hit &hit) {
    ASSERT(context.debug_text);
    context.debug_text->SetText(hit.path.ToString());
}

}  // namespace Debug

