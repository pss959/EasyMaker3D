#pragma once

#include <ion/gfx/image.h>

#include "Base/Memory.h"
#include "Enums/GripGuideType.h"
#include "Enums/Hand.h"
#include "Items/GripGuide.h"
#include "Math/Types.h"
#include "SG/Node.h"

namespace Parser { class Registry; }
namespace SG {
DECL_SHARED_PTR(Line);
DECL_SHARED_PTR(Shape);
}

DECL_SHARED_PTR(Controller);

/// The Controller class represents a VR controller, providing an interface to
/// interaction and feedback.
///
/// \ingroup Items
class Controller : public SG::Node {
  public:
    /// This struct represents the data necessary for replacing the default
    /// controller model with a custom one.
    struct CustomModel {
        SG::ShapePtr       shape;          ///< Shape with controller geometry.
        ion::gfx::ImagePtr texture_image;  ///< Texture image applied to shape.
    };

    /// Sets the hand this controller is for. Assumes Hand::kRight by default.
    void SetHand(Hand hand);

    /// Returns the hand this controller is for.
    Hand GetHand() const { return hand_; }

    /// Replaces the default model with the given CustomModel. The custom model
    /// will be scaled to approximately the size of the default model.
    void UseCustomModel(const CustomModel &custom_model);

    /// Sets the type of grip guide geometry to display. It is
    /// GripGuideType::kNone by default.
    void SetGripGuideType(GripGuideType type);

    /// Returns the current grip guide type.
    GripGuideType GetGripGuideType() const {
        return cur_guide_->GetGripGuideType();
    }

    /// Shows or hides the laser pointer.
    void ShowPointer(bool show);

    /// Shows or hides the grip feedback.
    void ShowGrip(bool show);

    /// Shows or hides pointer hover highlight (a small sphere at the
    /// intersection point). If show is true, the feedback is shown at the
    /// given point (in local coordinates).
    void ShowPointerHover(bool show, const Point3f &pt);

    /// Shows or hides a grip hover highlight (a connection from the current
    /// grip guide to a point on a widget to interact with). If show is true,
    /// the highlight is drawn to the given point (in local coordinates) with
    /// the given color.
    void ShowGripHover(bool show, const Point3f &pt, const Color &color);

    /// Lets the Controller react to an activation or deactivation of a pinch
    // or grip.
    void ShowActive(bool is_active, bool is_grip);

    /// Returns the direction of the controller guide. When the controller grip
    /// is not active, the direction is based on the Hand and the current
    /// GripGuideType. When it is active, it always points away from the palm.
    Vector3f GetGuideDirection() const;

    virtual void PostSetUpIon() override;

  protected:
    Controller() {}

  private:
    /// Hand this controller is for.
    Hand hand_ = Hand::kRight;

    /// All GripGuide children.
    std::vector<GripGuidePtr> guides_;

    /// Parent node for all guides.
    SG::NodePtr  guide_parent_;

    /// Current Guide displayed.
    GripGuidePtr cur_guide_;

    /// Root Node used to show the pointer and highlight.
    SG::NodePtr pointer_node_;

    /// Root Node used to show the grip guide and highlight.
    SG::NodePtr grip_node_;

    /// Node used to display pointer hover highlights.
    SG::NodePtr pointer_hover_node_;

    /// Node used to display grip hover highlights.
    SG::NodePtr grip_hover_node_;

    /// Line shape in the node used to display grip hover highlights.
    SG::LinePtr grip_hover_line_;

    /// This is set to true while grip dragging.
    bool is_grip_dragging = false;

    /// Rotates the guides for the left controller.
    void RotateGuides_();

    friend class Parser::Registry;
};
