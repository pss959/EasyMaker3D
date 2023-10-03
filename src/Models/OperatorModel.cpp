#include "Models/OperatorModel.h"

#include <ion/math/transformutils.h>

#include "Math/MeshUtils.h"

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
    SetCenterOffset(Vector3f(ComputeMeshBounds(mesh).GetCenter()));

    // Recenter the mesh.
    return CenterMesh(mesh);
}

void OperatorModel::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    // Skip over ParentModel so that children are not copied.
    Model::CopyContentsFrom(from, is_deep);
}
