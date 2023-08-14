#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Math/Types.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangePlaneCommand);

/// ChangePlaneCommand is a base class for commands used to modify a Plane in
/// one or more Models.
///
/// \ingroup Commands
class ChangePlaneCommand : public MultiModelCommand {
  public:
    /// Sets the Plane (in stage coordinates).
    void SetPlane(const Plane &plane);

    /// Returns the Plane (in stage coordinates). The default is the XY plane.
    const Plane & GetPlane() const { return plane_; }

  protected:
    ChangePlaneCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Plane> plane_;
    ///@}

    friend class Parser::Registry;
};
