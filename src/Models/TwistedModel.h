#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Math/Twist.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TwistedModel);

/// TwistedModel is a derived ConvertedModel class that represents a Model that
/// has been twisted by some angle around an arbitrary axis through an
/// arbitrary point. The point and axis are specified in object coordinates of
/// the operand Model.
///
/// \ingroup Models
class TwistedModel : public ConvertedModel {
  public:
    /// Sets the twist parameters.
    void SetTwist(const Twist &twist);

    /// Returns the current Twist.
    const Twist & GetTwist() const { return twist_; }

    virtual bool CanSetComplexity() const { return true; }

  protected:
    TwistedModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

    virtual TriMesh ConvertMesh(const TriMesh &mesh) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<Point3f>  center_;
    Parser::TField<Vector3f> axis_;
    Parser::TField<Anglef>   angle_;
    ///@}

    /// Twist used to create the model.
    Twist twist_;

    /// Caches the operand Model mesh split into slices based on complexity and
    /// the current Twist axis.
    TriMesh  split_mesh_;

    /// Complexity used to create #split_mesh_.
    float    split_complexity_ = -1;

    /// Axis used to create #split_mesh_.
    Vector3f split_axis_{0, 0, 0};

    friend class Parser::Registry;
};
