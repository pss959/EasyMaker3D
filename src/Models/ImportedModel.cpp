#include "Models/ImportedModel.h"

#include "Math/MeshBuilding.h" // XXXX

void ImportedModel::SetPath(const std::string &path) {
    path_ = path;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh ImportedModel::BuildMesh() {
    std::cerr << "XXXX " << GetDesc() << " importing from '"
              << GetPath() << "'\n";
    return BuildBoxMesh(Vector3f(2, 2, 2));
}
