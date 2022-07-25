#include "Models/ImportedModel.h"

#include "Base/Tuning.h"
#include "IO/STLReader.h"
#include "Items/UnitConversion.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"

ImportedModel::ImportedModel() :
    unit_conversion_(UnitConversion::CreateDefault()) {
}

void ImportedModel::AddFields() {
    AddModelField(path_.Init("path"));

    Model::AddFields();
}

void ImportedModel::SetUnitConversion(const UnitConversion &conv) {
    unit_conversion_->CopyFrom(conv);
}

void ImportedModel::SetPath(const std::string &path) {
    path_ = path;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh ImportedModel::BuildMesh() {
    import_error_.clear();

    TriMesh mesh;
    if (! path_.WasSet()) {
        import_error_ = "Import path was never set";
    }
    else {
        mesh = ReadSTLFile(GetPath(), unit_conversion_->GetFactor(),
                           import_error_);
    }

    if (mesh.GetTriangleCount() == 0) {
        // There was an error: use the placeholder tetrahedron mesh.
        ASSERT(! import_error_.empty());
        mesh = BuildTetrahedronMesh(2 * TK::kInitialModelScale);
    }

    return mesh;
}

bool ImportedModel::ValidateMesh(std::string &reason) const {
    // If there was an import error, return it.
    if (! import_error_.empty()) {
        reason = import_error_;
        return false;
    }
    return Model::ValidateMesh(reason);
}
