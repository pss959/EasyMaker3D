#include "Models/ImportedModel.h"

#include "Base/Tuning.h"
#include "IO/STLReader.h"
#include "Items/UnitConversion.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"

ImportedModel::ImportedModel() :
    unit_conversion_(UnitConversion::CreateWithUnits(
                         UnitConversion::Units::kCentimeters,
                         UnitConversion::Units::kCentimeters)) {
}

void ImportedModel::AddFields() {
    AddModelField(path_.Init("path"));

    PrimitiveModel::AddFields();
}

void ImportedModel::SetUnitConversion(const UnitConversion &conv) {
    unit_conversion_->CopyFrom(conv);
}

void ImportedModel::SetPath(const std::string &path) {
    path_ = path;
    ProcessChange(SG::Change::kGeometry, *this);

    // Clear any previous error; it does not relate to the new path.
    import_error_.clear();
}

TriMesh ImportedModel::BuildMesh() {
    import_error_.clear();

    TriMesh mesh;
    const std::string &path = GetPath();

    // The path will be empty if this Model was just created or there is an
    // error in the session file it was read from. In either case, treat this
    // as an error so the dummy tetrahedron mesh is used and is considered
    // invalid. Otherwise, try to import the mesh; it will be empty if there is
    // an error.
    if (path.empty())
        import_error_ = "Import path was never set";
    else
        mesh = CenterMesh(ReadSTLFile(path, unit_conversion_->GetFactor(),
                                      import_error_));

    // If there is no mesh for any reason, use a dummy tetrahedron mesh.
    if (mesh.GetTriangleCount() == 0) {
        ASSERT(! import_error_.empty());
        mesh = BuildTetrahedronMesh(2 * TK::kInitialModelScale);
    }
    else {
        was_loaded_successfully_ = true;
    }

    return mesh;
}

bool ImportedModel::ValidateMesh(TriMesh &mesh, std::string &reason) {
    // If there was an import error, return it.
    if (! import_error_.empty()) {
        reason = import_error_;
        return false;
    }
    // Otherwise, let the base class check.
    return PrimitiveModel::ValidateMesh(mesh, reason);
}
