#pragma once

#include <memory>

#include "Enums/GripGuideType.h"

namespace Parser { class Registry; }

/// Grippable is an abstract base class for nodes that implement VR controller
/// grip interaction in some way. There are several pure virtual functions that
/// derived classes must implement.
class Grippable : public SG::Node {
  public:
    /// Returns true if the Grippable is currently enabled.
    virtual bool IsGrippableEnabled() const = 0;

    /// This is given a GripInfo instance with just the Event filled in. The
    /// Grippable should set the other fields in the instance with the correct
    /// grip hover target.
    virtual void UpdateGripInfo(GripInfo &info) const = 0;

    //! Returns the GripGuideType that should be used to hover-highlight the
    // Grippable's interaction.
    virtual GripGuideType GetGripGuideType() const = 0;

  protected:
    Grippable() {}

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<Grippable> GrippablePtr;
