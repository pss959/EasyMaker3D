#pragma once

#include "Commands/MultiModelCommand.h"
#include "Math/Types.h"
#include "Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeMirrorCommand);

/// ChangeMirrorCommand is used to add a mirrorping Plane in one or more
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

  protected:
    ChangeMirrorCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Plane> plane_{"plane"};
    ///@}

    friend class Parser::Registry;
};
