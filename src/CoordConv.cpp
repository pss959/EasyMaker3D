#include "CoordConv.h"

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "SG/Node.h"

Matrix4f CoordConv::GetObjectToWorldMatrix(const SG::NodePath &path) const {
    Matrix4f m = Matrix4f::Identity();
    for (auto &node: path)
        m *= node->GetModelMatrix();
    return m;
}

Matrix4f CoordConv::GetWorldToObjectMatrix(const SG::NodePath &path) const {
    return ion::math::Inverse(GetObjectToWorldMatrix(path));
}

Matrix4f CoordConv::GetLocalToWorldMatrix(const SG::NodePath &path) const {
    Matrix4f m = Matrix4f::Identity();
    // Skip the last Node in the NodePath so its transforms are not included.
    for (auto &node: path)
        if (node != path.back())
            m *= node->GetModelMatrix();
    return m;
}

Matrix4f CoordConv::GetWorldToLocalMatrix(const SG::NodePath &path) const {
    return ion::math::Inverse(GetLocalToWorldMatrix(path));
}

Matrix4f CoordConv::GetObjectToStageMatrix(const SG::NodePath &path) const {
    return GetObjectToWorldMatrix(path) * GetWorldToStageMatrix();
}

Matrix4f CoordConv::GetStageToObjectMatrix(const SG::NodePath &path) const {
    return GetStageToWorldMatrix() * GetWorldToObjectMatrix(path);
}

Matrix4f CoordConv::GetLocalToStageMatrix(const SG::NodePath &path) const {
    return GetLocalToWorldMatrix(path) * GetWorldToStageMatrix();
}

Matrix4f CoordConv::GetStageToLocalMatrix(const SG::NodePath &path) const {
    return GetStageToWorldMatrix() * GetWorldToLocalMatrix(path);
}

Matrix4f CoordConv::GetStageToWorldMatrix() const {
    ASSERT(! stage_path_.empty());
    // Note that we use object coords for the stage path to convert to world
    // coordinates so that the stage transformations are included.
    return GetObjectToWorldMatrix(stage_path_);
}

Matrix4f CoordConv::GetWorldToStageMatrix() const {
    return ion::math::Inverse(GetStageToWorldMatrix());
}

#define CONVERSION_FUNC1_(type, name, matrix_func)                      \
type CoordConv::name(const SG::NodePath &path, const type &t) const {   \
    return matrix_func(path) * t;                                       \
}

#define CONVERSION_FUNC2_(type, name, matrix_func)                      \
type CoordConv::name(const type &t) const {                             \
    return matrix_func() * t;                                           \
}

CONVERSION_FUNC1_(Vector3f, ObjectToWorld, GetObjectToWorldMatrix)
CONVERSION_FUNC1_(Vector3f, WorldToObject, GetWorldToObjectMatrix)
CONVERSION_FUNC1_(Vector3f, LocalToWorld,  GetLocalToWorldMatrix)
CONVERSION_FUNC1_(Vector3f, WorldToLocal,  GetWorldToLocalMatrix)
CONVERSION_FUNC1_(Vector3f, ObjectToStage, GetObjectToStageMatrix)
CONVERSION_FUNC1_(Vector3f, StageToObject, GetStageToObjectMatrix)
CONVERSION_FUNC1_(Vector3f, LocalToStage,  GetLocalToStageMatrix)
CONVERSION_FUNC1_(Vector3f, StageToLocal,  GetStageToLocalMatrix)
CONVERSION_FUNC2_(Vector3f, StageToWorld,  GetStageToWorldMatrix)
CONVERSION_FUNC2_(Vector3f, WorldToStage,  GetWorldToStageMatrix)

CONVERSION_FUNC1_(Point3f,  ObjectToWorld, GetObjectToWorldMatrix)
CONVERSION_FUNC1_(Point3f,  WorldToObject, GetWorldToObjectMatrix)
CONVERSION_FUNC1_(Point3f,  LocalToWorld,  GetLocalToWorldMatrix)
CONVERSION_FUNC1_(Point3f,  WorldToLocal,  GetWorldToLocalMatrix)
CONVERSION_FUNC1_(Point3f,  ObjectToStage, GetObjectToStageMatrix)
CONVERSION_FUNC1_(Point3f,  StageToObject, GetStageToObjectMatrix)
CONVERSION_FUNC1_(Point3f,  LocalToStage,  GetLocalToStageMatrix)
CONVERSION_FUNC1_(Point3f,  StageToLocal,  GetStageToLocalMatrix)
CONVERSION_FUNC2_(Point3f,  StageToWorld,  GetStageToWorldMatrix)
CONVERSION_FUNC2_(Point3f,  WorldToStage,  GetWorldToStageMatrix)
