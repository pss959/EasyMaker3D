#include "Models/OperatorModel.h"

#include "Math/MeshUtils.h"

const Vector3f & OperatorModel::GetCenterOffset(bool update_first) const {
    // Make sure the mesh and offset are up to date if requested.
    if (update_first)
        GetMesh();

    return center_offset_;
}

bool OperatorModel::ProcessChange(SG::Change change, const Object &obj) {
    if (! ParentModel::ProcessChange(change, obj))
        return false;

    // A change in transform to a child or other descendent should cause the
    // mesh to be rebuilt.
    if (change == SG::Change::kTransform && &obj != this)
        MarkMeshAsStale();
    return true;
}

TriMesh OperatorModel::BuildMesh() {
    // Let the derived class build the mesh.
    TriMesh mesh = BuildMeshFromOperands();

    // Save the center as the offset.
    center_offset_ = Vector3f(ComputeMeshBounds(mesh).GetCenter());

    // Recenter the mesh.
    return CenterMesh(mesh);
}
