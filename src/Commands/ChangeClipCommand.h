﻿#pragma once

#include "Commands/MultiModelCommand.h"
#include "Math/Types.h"
#include "Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeClipCommand);

/// ChangeClipCommand is used to add a clipping Plane in one or more
/// ClippedModel instances.
///
/// \ingroup Commands
class ChangeClipCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the Plane (in stage coordinates) to clip to. The side the Plane
    /// normal points to is clipped away.
    void SetPlane(const Plane &plane);

    /// Returns the Plane (in stage coordinates) to clip to.
    const Plane & GetPlane() const { return plane_; }

  protected:
    ChangeClipCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Plane> plane_{"plane"};
    ///@}

    friend class Parser::Registry;
};
