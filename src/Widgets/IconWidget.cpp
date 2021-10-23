#include "IconWidget.h"

#include "SG/ImportedShape.h"

void IconWidget::AddFields() {
    AddField(import_path_);
    PushButtonWidget::AddFields();
}

bool IconWidget::IsValid(std::string &details) {
    if (! PushButtonWidget::IsValid(details))
        return false;

    // Always set use_bounds_proxy to true for icons.
    SetUseBoundsProxy(true);

    return true;
}

ion::gfx::NodePtr IconWidget::SetUpIon(
    const SG::IonContextPtr &ion_context,
    const std::vector<ion::gfx::ShaderProgramPtr> &programs) {
    // Add imported shape before letting the base class set up everything.
    if (! added_imported_shape_) {
        const std::string &path = GetImportPath();
        if (! path.empty())
            AddShape(SG::ImportedShape::CreateFrom(
                         path, SG::ImportedShape::NormalType::kFaceNormals));
        added_imported_shape_ = true;
    }

    return PushButtonWidget::SetUpIon(ion_context, programs);
}
