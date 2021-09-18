#include "Models/BoxModel.h"

#include "Math/MeshBuilding.h"

TriMesh BoxModel::BuildMesh() {
    return BuildBoxMesh(Vector3f(2, 2, 2));
}
