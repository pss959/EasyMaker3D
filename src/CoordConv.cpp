#include "CoordConv.h"

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "SG/Node.h"

CoordConv::CoordConv(const SG::NodePath &path) : path_(path) {
    ASSERT(! path_.empty());
}

Matrix4f CoordConv::GetObjectToRootMatrix() const {
    ASSERT(! path_.empty());
    Matrix4f m = Matrix4f::Identity();
    for (auto &node: path_)
        m *= node->GetModelMatrix();
    return m;
}

Matrix4f CoordConv::GetRootToObjectMatrix() const {
    return ion::math::Inverse(GetObjectToRootMatrix());
}

Matrix4f CoordConv::GetLocalToRootMatrix() const {
    ASSERT(! path_.empty());
    Matrix4f m = Matrix4f::Identity();
    // Skip the last Node in the NodePath so its transforms are not included.
    for (auto &node: path_)
        if (node != path_.back())
            m *= node->GetModelMatrix();
    return m;
}

Matrix4f CoordConv::GetRootToLocalMatrix() const {
    return ion::math::Inverse(GetLocalToRootMatrix());
}

#define CONVERSION_FUNC_(type, name, matrix_func)                             \
type CoordConv::name(const type &t) const { return matrix_func() * t; }

CONVERSION_FUNC_(Vector3f, ObjectToRoot, GetObjectToRootMatrix)
CONVERSION_FUNC_(Vector3f, RootToObject, GetRootToObjectMatrix)
CONVERSION_FUNC_(Vector3f, LocalToRoot,  GetLocalToRootMatrix)
CONVERSION_FUNC_(Vector3f, RootToLocal,  GetRootToLocalMatrix)

CONVERSION_FUNC_(Point3f,  ObjectToRoot, GetObjectToRootMatrix)
CONVERSION_FUNC_(Point3f,  RootToObject, GetRootToObjectMatrix)
CONVERSION_FUNC_(Point3f,  LocalToRoot,  GetLocalToRootMatrix)
CONVERSION_FUNC_(Point3f,  RootToLocal,  GetRootToLocalMatrix)
