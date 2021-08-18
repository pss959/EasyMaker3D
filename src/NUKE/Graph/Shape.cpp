#include "Graph/Shape.h"

namespace Graph {

Shape::Shape(const ion::gfx::ShapePtr &ion_shape) {
    i_shape_ = ion_shape;
}

void Shape::SetName_(const std::string &name) {
    Object::SetName_(name);
    i_shape_->SetLabel(name);
}

}  // namespace Graph
