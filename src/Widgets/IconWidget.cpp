#include "IconWidget.h"

#include "SG/ImportedShape.h"

void IconWidget::AddFields() {
    AddField(action_);
    AddField(import_path_);
    PushButtonWidget::AddFields();
}

void IconWidget::AllFieldsParsed(bool is_template) {
    // Always set use_bounds_proxy to true for icons.
    SetUseBoundsProxy(true);
}

void IconWidget::PreSetUpIon() {
    // Use special Icon colors.
    SetColorNamePrefix("Icon");

    // Add imported shape before letting the base class set up everything.
    if (! added_imported_shape_) {
        const std::string &path = GetImportPath();
        if (! path.empty()) {
            auto shape = SG::ImportedShape::CreateFrom(
                path, SG::ImportedShape::NormalType::kFaceNormals);
            AddShape(shape);
        }
        added_imported_shape_ = true;
    }

    PushButtonWidget::PreSetUpIon();
}

