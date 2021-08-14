#include "Graph/Shape.h"

namespace Graph {

void Shape::SetName_(const std::string &name) {
    Object::SetName_(name);
    i_shape_->SetLabel(name);
}

}  // namespace Graph
