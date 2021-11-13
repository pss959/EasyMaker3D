#pragma once

#include <memory>

#include "Enums/GripGuideType.h"
#include "Items/GripGuide.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "SG/Line.h"

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

    /// Shows a grip hover highlight to the given point with the given color.
    void ShowGripHover(const Point3f &pt, const Color &color);

    /// Hides any current grip hover highlight.
    void HideGripHover();

    virtual void PostSetUpIon() override;

  protected:
    Controller() {}

  private:
    /// All GripGuide children.
    std::vector<GripGuidePtr> guides_;

    /// Current Guide displayed.
    GripGuidePtr cur_guide_;

    /// Node used to display grip hover highlights.
    SG::NodePtr grip_hover_node_;

    /// Line shape in the node used to display grip hover highlights.
    SG::LinePtr grip_hover_line_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<Controller> ControllerPtr;
