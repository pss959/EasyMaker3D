#pragma once

#include <vector>

#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(BuildVolume);

/// The BuildVolume class encapsulates the visible build volume in the
/// scene. It provides an interface for turning it on and off and setting its
/// size.
///
/// \ingroup Items
class BuildVolume : public SG::Node {
  public:
    /// Sets the size of the build volume.
    void SetSize(const Vector3f &size) { size_ = size; }

    /// Returns the current size of the build volume.
    const Vector3f & GetSize() const { return size_; }

    /// Activates or deactivates the build volume.
    void Activate(bool is_active);

    /// Returns true if the build volume is active.
    bool IsActive() const { return IsEnabled(); }

  protected:
    BuildVolume() {}

  private:
    Vector3f size_{1, 1, 1};

    friend class Parser::Registry;
};
