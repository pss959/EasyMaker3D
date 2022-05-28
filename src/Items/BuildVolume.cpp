#include "Items/BuildVolume.h"

#include "Util/Assert.h"

void BuildVolume::Activate(bool is_active) {
    if (is_active) {
        SetScale(size_);
        SetTranslation(Vector3f(0, .5f * size_[1], 0));
    }
    SetEnabled(is_active);
}
