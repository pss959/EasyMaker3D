#include "BeveledMesh.h"

BeveledMesh::BeveledMesh(const TriMesh &mesh, const Anglef &max_angle) {
    original_mesh_ = mesh;
    max_angle_     = max_angle;

    // XXXX
    result_mesh_ = mesh; // XXXX
}
