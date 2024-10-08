//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "SG/Camera.h"
#include "SG/Object.h"
#include "Util/Memory.h"

class Frustum;

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(Gantry);

/// The Gantry class is a special-purpose scene object that contains any number
/// of Camera objects and can move them up and down.
///
/// \ingroup SG
class Gantry : public Object {
  public:
    /// Returns the cameras managed by the gantry.
    const std::vector<CameraPtr> & GetCameras() const { return cameras_; }

    /// Sets the gantry height. It starts at 0.
    void SetHeight(float height);

    /// Returns the current height.
    float GetHeight() const { return height_; }

  protected:
    Gantry() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Camera> cameras_;
    ///@}

    float height_ = 0;   ///< Current height.

    friend class Parser::Registry;
};

}  // namespace SG
