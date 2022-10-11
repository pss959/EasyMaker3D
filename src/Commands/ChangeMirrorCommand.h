#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Math/Types.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeMirrorCommand);

/// ChangeMirrorCommand is used to add a mirroring Plane in one or more
/// MirroredModel instances.
///
/// \ingroup Commands
class ChangeMirrorCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the Plane (in stage coordinates) to mirror across.
    void SetPlane(const Plane &plane);

    /// Returns the Plane (in stage coordinates) to mirror across.
    const Plane & GetPlane() const { return plane_; }

    /// Returns a flag indicating whether secondary Models are mirrored in place
    /// or relative to the primary model.
    bool IsInPlace() const { return is_in_place_; }

    /// Sets a flag indicating whether secondary Models are mirrored in place
    /// or relative to the primary model.
    void SetInPlace(bool in_place) { is_in_place_ = in_place; }

  protected:
    ChangeMirrorCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Plane> plane_;
    Parser::TField<bool>  is_in_place_;
    ///@}

    friend class Parser::Registry;
};
