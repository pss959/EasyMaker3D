#include "Widgets/IconWidget.h"

#include "Math/Linear.h"
#include "SG/ImportedShape.h"

void IconWidget::AddFields() {
    AddField(action_.Init("action", Action::kNone));
    AddField(import_path_.Init("import_path"));

    PushButtonWidget::AddFields();
}

void IconWidget::CreationDone() {
    PushButtonWidget::CreationDone();

    if (! IsTemplate()) {
        // Always set use_bounds_proxy to true for icons.
        SetUseBoundsProxy(true);

        // Use special Icon colors.
        SetColorNamePrefix("Icon");

        // Add imported shape.
        const std::string &path = GetImportPath();
        if (! path.empty()) {
            auto shape = SG::ImportedShape::CreateFrom(
                path, SG::ImportedShape::NormalType::kFaceNormals);
            AddShape(shape);
        }
    }
}

void IconWidget::FitIntoCube(float size, const Point3f &center) {
    FitNodeIntoCube(*this, size, center);
}

void IconWidget::FitNodeIntoCube(SG::Node &node,
                                 float size, const Point3f &center) {
    // Scale so that the largest dimension of the icon just fits into the cube.
    const Bounds &bounds = node.GetBounds();
    const int max_index = GetMaxElementIndex(bounds.GetSize());
    const float scale = size / bounds.GetSize()[max_index];
    node.SetUniformScale(scale);

    // Position in Z so that the front of the icon is flush with the front of
    // the cube.
    const float z = center[2] + .5f * (size - scale * bounds.GetSize()[2]);
    node.SetTranslation(Vector3f(center[0], center[1], z));
}
