//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <ion/gfx/image.h>

#include "Enums/GripGuideType.h"
#include "Enums/Hand.h"
#include "Enums/Trigger.h"
#include "Items/GripGuide.h"
#include "Math/Color.h"
#include "Math/ModelMesh.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }
namespace SG { DECL_SHARED_PTR(Tube); }

DECL_SHARED_PTR(Controller);
DECL_SHARED_PTR(RadialMenu);

/// The Controller class represents a VR controller, providing an interface to
/// interaction and feedback.
///
/// \ingroup Items
class Controller : public SG::Node {
  public:
    using VibrateFunc = std::function<void(float)>;

    /// This struct represents the data necessary for replacing the default
    /// controller model with a custom one.
    struct CustomModel {
        ModelMesh          mesh;           ///< Mesh with controller model.
        ion::gfx::ImagePtr texture_image;  ///< Texture image applied to mesh.

        /// \name Connection points
        /// Each of these is the position of a vertex that is used to attach
        /// a corresponding affordance.
        ///@{
        Point3f pointer_pos;  ///< Starting point of laser pointer.
        Point3f grip_pos;     ///< Attachment point of grip guide affordance.
        Point3f touch_pos;    ///< Attachment point of touch affordance.
        ///@}

        CustomModel() { pointer_pos = grip_pos = touch_pos = Point3f::Zero(); }
    };

    /// Sets the hand this controller is for. Assumes Hand::kRight by default.
    void SetHand(Hand hand);

    /// Returns the hand this controller is for.
    Hand GetHand() const { return hand_; }

    /// Sets a function to invoke to vibrate the controller, if available. The
    /// function is passed the duration in seconds.
    void SetVibrateFunc(const VibrateFunc &func) { vibrate_func_ = func; }

    /// Replaces the default model with the given CustomModel. The custom model
    /// will be scaled to approximately the size of the default model.
    void UseCustomModel(const CustomModel &custom_model);

    /// \name Grip Guides
    ///@{

    /// Sets the type of grip guide geometry to display. It is
    /// GripGuideType::kNone by default.
    void SetGripGuideType(GripGuideType type);

    /// Returns the current grip guide type.
    GripGuideType GetGripGuideType() const;

    /// Returns the direction of the controller guide. When the controller grip
    /// is not active, the direction is based on the Hand and the current
    /// GripGuideType. When it is active, it always points away from the palm.
    Vector3f GetGuideDirection() const;

    ///@}

    ///
    /// \name Touch Mode
    ///@{

    /// Puts the controller in touch mode or reverts to regular mode. When in
    /// touch mode, the touch affordance is shown instead of the laser pointer.
    void SetTouchMode(bool in_touch_mode);

    /// Returns true if the controller is in touch mode.
    bool IsInTouchMode() const { return is_in_touch_mode_; }

    /// Returns the offset from the controller position to the touch
    /// affordance. This should be added to the controller position when in
    /// touch mode to get the touch position.
    const Vector3f GetTouchOffset() const { return touch_offset_; }

    /// Returns the radius to use for touch interaction. The radius is based on
    /// the size of the sphere of the touch affordance for consistency.
    float GetTouchRadius() const { return touch_radius_; }

    ///@}

    /// \name Affordance and Feedback Control
    ///@{

    /// Shows or hides all current affordances for the Controller. This should
    /// not be called while the Controller is triggered.
    void ShowAll(bool show);

    /// Sets trigger mode for the Controller. When is_triggered is true, only
    /// the affordances for the given Trigger mode are shown. When it is false,
    /// all current affordances are shown.
    void SetTriggerMode(Trigger trigger, bool is_triggered);

    /// Shows or hides pointer hover highlight (a small sphere at the
    /// intersection point). If show is true, the feedback is shown at the
    /// given point (in local coordinates).
    void ShowPointerHover(bool show, const Point3f &pt);

    /// Shows or hides a grip hover highlight (a connection from the current
    /// grip guide to a point on a widget to interact with). If show is true,
    /// the highlight is drawn to the given point (in local coordinates) with
    /// the given color.
    void ShowGripHover(bool show, const Point3f &pt, const Color &color);

    /// Shows that the Controller has begun or ended a touch operation.
    void ShowTouch(bool is_start);

    /// Vibrates the controller (if possible) for the given duration.
    void Vibrate(float seconds);

    ///@}

    /// \name Attaching Objects
    ///@{

    /// Attaches the given object to the Controller so that it moves with
    /// it. The given offset vector is used; the X is adjusted based on the
    /// Controller's Hand. Note that this modifies the rotation and translation
    /// of the object.
    void AttachObject(const SG::NodePtr &object, const Vector3f &offset);

    /// Detaches an attached object. Asserts if it is not currently attached.
    void DetachObject(const SG::NodePtr &object);

    /// Attaches the given RadialMenu.
    void AttachRadialMenu(const RadialMenuPtr &menu);

    ///@}

    virtual void PostSetUpIon() override;

    /// If this is using a CustomModel, sets the given TriMesh and image to
    /// the mesh and texture image fields for the CustomModel and returns true.
    /// Otherwise, just returns false.
    bool GetCustomModelData(ModelMesh &mesh, ion::gfx::ImagePtr &image) const;

  protected:
    Controller() {}

  private:
    /// Hand this controller is for.
    Hand hand_ = Hand::kRight;

    /// Function used for haptic vibration.
    VibrateFunc vibrate_func_;

    /// CustomModel; will have an empty mesh if not set.
    CustomModel custom_model_;

    /// All GripGuide children.
    std::vector<GripGuidePtr> guides_;

    /// Parent node for all guides.
    SG::NodePtr  guide_parent_;

    /// Current Guide displayed.
    GripGuidePtr cur_guide_;

    /// Node representing the Controller model geometry.
    SG::NodePtr model_node_;

    /// Node used to show the touch affordance.
    SG::NodePtr touch_node_;

    /// Node used to show the touch affordance tip.
    SG::NodePtr touch_tip_node_;

    /// Root Node used to show the pointer and highlight.
    SG::NodePtr pointer_node_;

    /// Root Node used to show the grip guide and highlight.
    SG::NodePtr grip_node_;

    /// Node used to display pointer hover highlights.
    SG::NodePtr pointer_hover_node_;

    /// Node used to display grip hover highlights.
    SG::NodePtr grip_hover_node_;

    /// Tube shape in the node for the laser pointer.
    SG::TubePtr pointer_hover_tube_;

    /// Tube shape in the node used to display grip hover highlights.
    SG::TubePtr grip_hover_tube_;

    /// Starting point of laser pointer.
    Point3f     pointer_start_point_{0, 0, 0};

    /// Offset from the controller position to the tip of the touch affordance
    /// position.
    Vector3f    touch_offset_{0, 0, 0};

    /// Size of the touch affordance sphere to be used as the touch radius.
    float touch_radius_ = 0;

    /// Set to true while in touch mode.
    bool is_in_touch_mode_ = false;

    /// This is set to true while grip dragging.
    bool is_grip_dragging_ = false;

    /// Shows or hides the affordance associated with the given Trigger.
    void ShowAffordance_(Trigger trigger, bool is_shown);

    /// Rotates the guides for the left controller.
    void RotateLeftGuides_();

    /// Updates items that depend on geometry. This is called for the default
    /// model and again if a custom model is loaded.
    void UpdateForGeometry_();

    /// Positions the hover guides to connect to a reasonable spot on the
    /// controller model. Rotates if this is the left controller.
    void PositionGuides_();

    /// Sets up the touch geometry.
    void SetUpForTouch_();

    /// Computes a taper value for a laser pointer or grip hover between two
    /// points.
    static float ComputeTaper_(const Point3f &p0, const Point3f &p1);

    friend class Parser::Registry;
};
