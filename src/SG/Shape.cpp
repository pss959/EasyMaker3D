#include "SG/Shape.h"

namespace SG {

void Shape::SetUpIon(IonContext &context) {
    if (! ion_shape_) {
        ion_shape_ = CreateIonShape();
        ion_shape_->SetLabel(GetName());
    }
}

}  // namespace SG
