#pragma once

#include <memory>

#include "Enums/GripGuideType.h"
#include "Items/GripGuide.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

/// The Controller class represents a VR controller, providing an interface to
/// interaction and feedback.
class Controller : public SG::Node {
  public:
    /// Sets the type of grip guide geometry to display. It is
    /// GripGuideType::kNone by default.
    void SetGripGuideType(GripGuideType type);

    /// Returns the current grip guide type.
    GripGuideType GetGripGuideType() const {
        return cur_guide_->GetGripGuideType();
    }

    /// Returns the hover attachment point for the current guide relative to
    /// the GripGuide.
    const Point3f & GetHoverPoint() const {
        return cur_guide_->GetHoverPoint();
    }

    virtual void PostSetUpIon() override;

  protected:
    Controller() {}

  private:
    /// All GripGuide children.
    std::vector<GripGuidePtr> guides_;

    /// Current Guide displayed.
    GripGuidePtr cur_guide_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<Controller> ControllerPtr;
