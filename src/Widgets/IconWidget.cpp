#include "IconWidget.h"

#include "SG/ImportedShape.h"

void IconWidget::AddFields() {
    AddField(action_);
    AddField(import_path_);
    PushButtonWidget::AddFields();
}

void IconWidget::CreationDone(bool is_template) {
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

    PushButtonWidget::CreationDone(is_template);
}

