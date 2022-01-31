#pragma once

#include "Math/Types.h"
#include "SG/NodePath.h"

/// The CoordConv class helps convert between various coordinate systems.
///
/// <em>Object coordinates</em> represent the coordinate system for a Node
/// before any of its local transform fields or any transform fields for
/// ancestors are applied.
///
/// <em>Local coordinates</em> represent the coordinate system for a Node after
/// applying its local transform fields, but not any transform fields for
/// ancestors are applied.
///
/// <em>Stage coordinates</em> are for objects that are placed on or near the
/// Stage. Note that the local transformations of the Stage itself (as when the
/// user scales and rotates it) are not applied to Stage coordinates except
/// when converting them to world coordinates. Stage coordinates are defined by
/// an SG::NodePath from the root Node of the scene to the Stage.
///
/// <em>World coordinates</em> are the coordinate system at the root node of
/// the scene.
class CoordConv {
  public:
    /// Sets a path from the root of the scene to the node that represents the
    /// Stage. This must be called before any conversions to and from Stage
    /// coordinates can be performed.
    void SetStagePath(const SG::NodePath &path) { stage_path_ = path; }

    /// \name Matrix Accessors.
    /// Each of these computes and returns a matrix that converts between two
    /// coordinate systems. A NodePath must be provided to define local,
    /// object, or world coordinates.
    ///@{

    /// Returns a matrix that converts from object to world coordinates.
    Matrix4f GetObjectToWorldMatrix(const SG::NodePath &path) const;

    /// Returns a matrix that converts from world to object coordinates.
    Matrix4f GetWorldToObjectMatrix(const SG::NodePath &path) const;

    /// Returns a matrix that converts from local to world coordinates.
    Matrix4f GetLocalToWorldMatrix(const SG::NodePath &path) const;

    /// Returns a matrix that converts from world to local coordinates.
    Matrix4f GetWorldToLocalMatrix(const SG::NodePath &path) const;

    /// Returns a matrix that converts from object to stage coordinates.
    Matrix4f GetObjectToStageMatrix(const SG::NodePath &path) const;

    /// Returns a matrix that converts from stage to object coordinates.
    Matrix4f GetStageToObjectMatrix(const SG::NodePath &path) const;

    /// Returns a matrix that converts from local to stage coordinates.
    Matrix4f GetLocalToStageMatrix(const SG::NodePath &path) const;

    /// Returns a matrix that converts from stage to local coordinates.
    Matrix4f GetStageToLocalMatrix(const SG::NodePath &path) const;

    /// Returns a matrix that converts from stage to world coordinates.
    Matrix4f GetStageToWorldMatrix() const;

    /// Returns a matrix that converts from world to stage coordinates.
    Matrix4f GetWorldToStageMatrix() const;

    ///@}

    /// \name Vector and Point Transformation Functions.

    /// Each of these transforms a 3D vector or point between coordinate
    /// systems. A NodePath must be provided to define local, object, or world
    /// coordinates.
    ///@{

    Vector3f ObjectToWorld(const SG::NodePath &path, const Vector3f &v) const;
    Vector3f WorldToObject(const SG::NodePath &path, const Vector3f &v) const;
    Vector3f LocalToWorld(const SG::NodePath &path, const Vector3f &v) const;
    Vector3f WorldToLocal(const SG::NodePath &path, const Vector3f &v) const;
    Vector3f ObjectToStage(const SG::NodePath &path, const Vector3f &v) const;
    Vector3f StageToObject(const SG::NodePath &path, const Vector3f &v) const;
    Vector3f LocalToStage(const SG::NodePath &path, const Vector3f &v) const;
    Vector3f StageToLocal(const SG::NodePath &path, const Vector3f &v) const;
    Vector3f StageToWorld(const Vector3f &v) const;
    Vector3f WorldToStage(const Vector3f &v) const;

    Point3f ObjectToWorld(const SG::NodePath &path, const Point3f &p) const;
    Point3f WorldToObject(const SG::NodePath &path, const Point3f &p) const;
    Point3f LocalToWorld(const SG::NodePath &path, const Point3f &p) const;
    Point3f WorldToLocal(const SG::NodePath &path, const Point3f &p) const;
    Point3f ObjectToStage(const SG::NodePath &path, const Point3f &p) const;
    Point3f StageToObject(const SG::NodePath &path, const Point3f &p) const;
    Point3f LocalToStage(const SG::NodePath &path, const Point3f &p) const;
    Point3f StageToLocal(const SG::NodePath &path, const Point3f &p) const;
    Point3f StageToWorld(const Point3f &p) const;
    Point3f WorldToStage(const Point3f &p) const;

    ///@}

  private:
    SG::NodePath stage_path_;
};
