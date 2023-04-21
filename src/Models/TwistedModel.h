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

    friend class Parser::Registry;
};
