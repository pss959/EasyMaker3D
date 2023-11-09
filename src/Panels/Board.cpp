#include "Panels/Board.h"

#include <stack>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Items/Controller.h"
#include "Items/Frame.h"
#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "Panels/Panel.h"
#include "Place/TouchInfo.h"
#include "SG/ColorMap.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"
#include "Widgets/Slider2DWidget.h"

// ----------------------------------------------------------------------------
// Board::Impl_ class.
// ----------------------------------------------------------------------------

class Board::Impl_ {
  public:
    /// The constructor is passed the root node to find all parts under.
    Impl_(SG::Node &root_node);
    void SetPathToRootNode(const SG::NodePath &path_to_root_node) {
        ASSERT(! path_to_root_node.empty());
        path_to_root_node_ = path_to_root_node;
    }
    void InitCanvas();
    void SetPanel(const PanelPtr &panel,
                  const BoardAgent::ResultFunc &result_func);
    void PushPanel(const PanelPtr &panel,
                   const BoardAgent::ResultFunc &result_func);
    bool PopPanel(const Str &result);
    size_t GetPanelCount() const { return panel_stack_.size(); }
    PanelPtr GetCurrentPanel() const;
    void SetDistanceScale(float scale) { distance_scale_ = scale; }
    void Show(bool shown);
    void UpdateSizeIfNecessary();
    void SetUpForTouch(const Point3f &cam_pos, const Vector3f &offset);
    void SetOrientation(const Vector3f &view_dir);
    void SetPosition(const Point3f &pos);
    bool IsGrippableEnabled() const {
        return is_move_enabled_ || is_size_enabled_;
    }
    void UpdateGripInfo(GripInfo &info);
    void ActivateGrip(Hand hand, bool is_active);
    WidgetPtr GetTouchedWidget(const Point3f &touch_pos,
                               float radius) const;

  private:
    /// Struct representing an active Panel. These are stored in a stack.
    struct PanelInfo_ {
        PanelPtr               panel;
        BoardAgent::ResultFunc result_func;
    };

    /// This struct represents the current grip state for a controller.
    struct GripState_ {
        /// Saves the slider that was last hovered in UpdateGripInfo().
        Slider2DWidgetPtr hovered_slider;

        /// This saves the part that was last hovered in UpdateGripInfo() so
        /// that the Size_() function knows what to do.
        SG::NodePtr       hovered_part;

        /// This saves the part that is currently being dragged so that it is
        /// the only one that is hovered.
        SG::NodePtr       dragged_part;

        /// True if the grip button is active.
        bool is_active = false;
    };

    SG::Node &root_node_;

    /// Path from the scene root to the root node; used for coordinate
    /// conversions.
    SG::NodePath path_to_root_node_;

    /// Set to true when set up for touch interaction.
    bool is_set_up_for_touch_ = false;

    // Parts.
    SG::NodePtr       canvas_;          ///< Canvas rectangle.
    Slider2DWidgetPtr xy_move_slider_;  ///< Move slider with handles on sides.
    Slider2DWidgetPtr xz_move_slider_;  ///< Move slider with handle in front.
    Slider2DWidgetPtr size_slider_;     ///< Size slider with handles at corners.
    FramePtr          frame_;           ///< Frame around the Board.

    /// All slider handles that can be grip-hovered. 4 handles for the
    /// xy_move_slider_, 1 for the xz_move_slider_, and 4 for the size_slider_.
    SG::NodePtr       handles_[4 + 1 + 4];

    /// Stack of active Panels.
    std::stack<PanelInfo_> panel_stack_;

    // Sizes.
    Vector2f          world_size_{0, 0};  ///< Board size in world coordinates.
    Vector2f          panel_size_{0, 0};  ///< Board size in panel coordinates.

    float             distance_scale_        = 1;
    bool              is_shown_              = false;
    bool              is_move_enabled_       = true;
    bool              is_size_enabled_       = true;
    bool              size_may_have_changed_ = false;
    Vector3f          start_pos_;               ///< Used for computing motion.

    // Grip state.
    GripState_        l_grip_state_;   ///< Grip state for left controller.
    GripState_        r_grip_state_;   ///< Grip state for right controller.

    /// Finds and stores all of the necessary parts.
    void FindParts_();

    /// Pushes a PanelInfo_ on the stack.
    void PushPanelInfo_(const PanelPtr &panel,
                        const BoardAgent::ResultFunc &result_func);

    /// Replaces one Panel with another. Either (but not both) may be null.
    void ReplacePanel_(const PanelPtr &cur_panel, const PanelPtr &new_panel,
                       bool is_pop);

    // Move and size slider callbacks.
    void MoveActivated_(bool is_xy, bool is_activation);
    void SizeActivated_(bool is_activation);
    void Move_(bool is_xy);
    void Size_();

    /// Updates the Board in response to a size change in the given Panel.
    void UpdateSizeFromPanel_(const Vector2f &panel_size);

    /// Updates the sizes of the canvas and the Frame.
    void UpdateCanvasAndFrame_();

    /// Updates the Board scale to compensate for touch positioning.
    void UpdateScaleForTouch_();

    /// Updates the positions of handles based on world_size_.
    void UpdateHandlePositions_();

    /// Sets the dragged part to the hovered part or to null.
    void SetDraggedPart_(bool use_hovered_part);

    /// If the guide direction in the given GripInfo is close to perpendicular
    /// to the Board and the current Panel in the board supports grip hovering,
    /// this asks the Panel to update the GripInfo. Returns true if the Panel
    /// was able to do the job.
    bool UpdatePanelGripInfo_(GripInfo &info);

    /// Returns the best part to grip hover based on the controller direction.
    void GetBestGripHoverPart_(const Vector3f &guide_direction,
                               GripState_ &state);

    /// Updates the GripInfo from the GripState_.
    void UpdateGripHover_(const GripState_ &state, GripInfo &info);

    /// Returns true if the Board is set up for touch interaction.
    bool IsSetUpForTouch_() const { return is_set_up_for_touch_; }
};

Board::Impl_::Impl_(SG::Node &root_node) : root_node_(root_node) {
    FindParts_();
}

void Board::Impl_::InitCanvas() {
    canvas_->SetBaseColor(SG::ColorMap::SGetColor("BoardCanvasColor"));
}

void Board::Impl_::SetPanel(const PanelPtr &panel,
                            const BoardAgent::ResultFunc &result_func) {
    ASSERT(panel);
    ASSERT(panel_stack_.empty());
    KLOG('g', root_node_.GetDesc() << " SetPanel to " << panel->GetDesc());
    PushPanelInfo_(panel, result_func);
    ReplacePanel_(nullptr, panel, false);
}

void Board::Impl_::PushPanel(const PanelPtr &panel,
                             const BoardAgent::ResultFunc &result_func) {
    ASSERT(panel);
    ASSERT(! panel_stack_.empty());

    KLOG('g', root_node_.GetDesc() << " PushPanel " << panel->GetDesc());

    const PanelPtr cur_panel = GetCurrentPanel();
    ASSERT(cur_panel);

    PushPanelInfo_(panel, result_func);
    ReplacePanel_(cur_panel, panel, false);
}

bool Board::Impl_::PopPanel(const Str &result) {
    ASSERT(! panel_stack_.empty());

    // Copy the info so the pop() does not affect the rest of the code.
    auto info = panel_stack_.top();
    panel_stack_.pop();
    const PanelPtr cur_panel = GetCurrentPanel();

    KLOG('g', root_node_.GetDesc() << " PopPanel " << info.panel->GetDesc()
         << " with result '" << result << "'");
    if (cur_panel) {
        KLOG('g', root_node_.GetDesc() << " restoring "
             << cur_panel->GetDesc());
    }

    ReplacePanel_(info.panel, cur_panel, true);

    if (info.result_func)
        info.result_func(result);

    return cur_panel.get();  // True if Board is not empty.
}


PanelPtr Board::Impl_::GetCurrentPanel() const {
    PanelPtr panel;
    if (! panel_stack_.empty())
        panel = panel_stack_.top().panel;
    return panel;
}

void Board::Impl_::Show(bool shown) {
    if (auto cur_panel = GetCurrentPanel())
        cur_panel->SetStatus(shown ? Panel::Status::kVisible :
                             Panel::Status::kUnattached);
    is_shown_ = shown;
}

void Board::Impl_::UpdateSizeIfNecessary() {
    if (auto cur_panel = GetCurrentPanel()) {
        if (size_may_have_changed_) {
            // Make sure the Panel has the correct size and update the Board
            // size to match if necessary.
            KLOG('q', root_node_.GetDesc() << "detected Panel size change");
            if (cur_panel->GetSize() != panel_size_)
                UpdateSizeFromPanel_(cur_panel->GetSize());
            size_may_have_changed_ = false;
        }
    }
}

void Board::Impl_::SetUpForTouch(const Point3f &cam_pos,
                                 const Vector3f &offset) {
    // Origin == disable.
    if (cam_pos == Point3f::Zero()) {
        is_set_up_for_touch_ = false;
        root_node_.SetTranslation(Vector3f::Zero());
    }
    else {
        ASSERT(cam_pos[2] != 0);

        // Center in X, stay even with the camera in Y, and use the computed Z.
        // Then add the offset.
        const Vector3f trans(0, cam_pos[1],
                             cam_pos[2] - TK::kBoardTouchDistance);
        root_node_.SetTranslation(trans + offset);

        if (! canvas_->GetBounds().IsEmpty())
            UpdateScaleForTouch_();

        is_set_up_for_touch_ = true;
    }
    std::cerr << "XXXX " << root_node_.GetName()
              << " trans = " << root_node_.GetTranslation()
              << " from cam pos " << cam_pos
              << "\n";
}

void Board::Impl_::SetOrientation(const Vector3f &view_dir) {
    // Do nothing if set up for touch interaction.
    if (! IsSetUpForTouch_())
        root_node_.SetRotation(
            Rotationf::RotateInto(-Vector3f::AxisZ(), view_dir));
}

void Board::Impl_::SetPosition(const Point3f &pos) {
    // Do nothing if set up for touch interaction.
    if (! IsSetUpForTouch_())
        root_node_.TranslateTo(pos);
}

void Board::Impl_::UpdateGripInfo(GripInfo &info) {
    auto &state = info.event.device == Event::Device::kLeftController ?
        l_grip_state_ : r_grip_state_;

    // This should not be called in the middle of a drag.
    ASSERT(! state.dragged_part);

    // Try asking the Panel first.
    if (UpdatePanelGripInfo_(info))
        return;

    // Otherwise, use the controller orientation to get the best part to hover.
    GetBestGripHoverPart_(info.guide_direction, state);
    UpdateGripHover_(state, info);
}

void Board::Impl_::ActivateGrip(Hand hand, bool is_active) {
    if (hand == Hand::kLeft)
        l_grip_state_.is_active = is_active;
    else
        r_grip_state_.is_active = is_active;
}

WidgetPtr Board::Impl_::GetTouchedWidget(const Point3f &touch_pos,
                                         float radius) const {
    WidgetPtr widget;
    if (auto panel = GetCurrentPanel()) {
        // Check for intersection with the Board's bounds (in world
        // coordinates) first for trivial reject.
        const auto world_bounds = TranslateBounds(root_node_.GetScaledBounds(),
                                                  root_node_.GetTranslation());
        float dist = 0;
        if (SphereBoundsIntersect(touch_pos, radius, world_bounds, dist)) {
            KLOG('U', "Touch on " << root_node_.GetDesc()
                 << " with " << panel->GetDesc());
            TouchInfo info;
            info.position = touch_pos;
            info.radius   = radius;
            info.root_node =
                Util::CreateTemporarySharedPtr<SG::Node>(&root_node_);
            widget = panel->GetTouchedPaneWidget(info);
        }
        else {
            KLOG('U', "Missed touch on " << root_node_.GetDesc()
                 << " with " << panel->GetDesc()
                 << ": " << touch_pos << " vs " << world_bounds);
        }
    }
    return widget;
}

void Board::Impl_::FindParts_() {
    ASSERT(! canvas_);

    // Find all of the necessary parts.
    canvas_ = SG::FindNodeUnderNode(root_node_, "Canvas");
    xy_move_slider_ =
        SG::FindTypedNodeUnderNode<Slider2DWidget>(root_node_, "XYMoveSlider");
    xz_move_slider_ =
        SG::FindTypedNodeUnderNode<Slider2DWidget>(root_node_, "XZMoveSlider");
    size_slider_ =
        SG::FindTypedNodeUnderNode<Slider2DWidget>(root_node_, "SizeSlider");
    frame_ = SG::FindTypedNodeUnderNode<Frame>(root_node_, "BoardFrame");

    // Set up the sliders.
    xy_move_slider_->GetActivation().AddObserver(
        this, [&](Widget &, bool is_act){ MoveActivated_(true, is_act); });
    xy_move_slider_->GetValueChanged().AddObserver(
        this, [&](Widget &, const Vector2f &){ Move_(true); });
    xz_move_slider_->GetActivation().AddObserver(
        this, [&](Widget &, bool is_act){ MoveActivated_(false, is_act); });
    xz_move_slider_->GetValueChanged().AddObserver(
        this, [&](Widget &, const Vector2f &){ Move_(false); });
    size_slider_->GetActivation().AddObserver(
        this, [&](Widget &, bool is_act){ SizeActivated_(is_act); });
    size_slider_->GetValueChanged().AddObserver(
        this, [&](Widget &, const Vector2f &){ Size_(); });

    // Don't track motion until activation.
    xy_move_slider_->GetValueChanged().EnableObserver(this, true);
    xz_move_slider_->GetValueChanged().EnableObserver(this, true);
    size_slider_->GetValueChanged().EnableObserver(this, true);

    // Access all handles that can be grip targets.
    handles_[0] = SG::FindNodeUnderNode(*xy_move_slider_, "Left");
    handles_[1] = SG::FindNodeUnderNode(*xy_move_slider_, "Right");
    handles_[2] = SG::FindNodeUnderNode(*xy_move_slider_, "Bottom");
    handles_[3] = SG::FindNodeUnderNode(*xy_move_slider_, "Top");
    handles_[4] = SG::FindNodeUnderNode(root_node_,       "Bar");
    handles_[5] = SG::FindNodeUnderNode(*size_slider_,    "BottomLeft");
    handles_[6] = SG::FindNodeUnderNode(*size_slider_,    "BottomRight");
    handles_[7] = SG::FindNodeUnderNode(*size_slider_,    "TopLeft");
    handles_[8] = SG::FindNodeUnderNode(*size_slider_,    "TopRight");
}

void Board::Impl_::PushPanelInfo_(const PanelPtr &panel,
                                  const BoardAgent::ResultFunc &result_func) {
    PanelInfo_ info;
    info.panel       = panel;
    info.result_func = result_func;
    panel_stack_.push(info);
}

void Board::Impl_::ReplacePanel_(const PanelPtr &cur_panel,
                                 const PanelPtr &new_panel, bool is_pop) {
    ASSERT(cur_panel || new_panel);

    if (cur_panel) {
        cur_panel->SetStatus(is_pop ? Panel::Status::kUnattached :
                             Panel::Status::kHidden);
        canvas_->RemoveChild(cur_panel);
        cur_panel->GetSizeChanged().RemoveObserver(this);
    }
    if (new_panel) {
        // Set the new panel to visible if the Board is shown.
        if (is_shown_)
            new_panel->SetStatus(Panel::Status::kVisible);
        canvas_->AddChild(new_panel);

        // Ask the Panel whether to show sliders.
        is_move_enabled_ = new_panel->IsMovable();
        is_size_enabled_ = new_panel->IsResizable();

        // If the Panel size was never set, set it now.
        if (new_panel->GetSize() == Vector2f::Zero()) {
            new_panel->SetDistanceScale(distance_scale_);
            new_panel->SetSize(new_panel->GetMinSize());
        }

        // Update the Board to the Panel's current size.
        UpdateSizeFromPanel_(new_panel->GetSize());

        // Track size changes.
        size_may_have_changed_ = false;
        new_panel->GetSizeChanged().AddObserver(
            this, [&](){ size_may_have_changed_ = true; });
    }
}

void Board::Impl_::MoveActivated_(bool is_xy, bool is_activation) {
    auto active_slider = is_xy ? xy_move_slider_ : xz_move_slider_;
    auto other_slider  = is_xy ? xz_move_slider_ : xy_move_slider_;

    if (is_activation) {
        // Save the current canvas translation.
        start_pos_ = canvas_->GetTranslation();

        // Turn off display of other handles.
        other_slider->SetEnabled(false);
        size_slider_->SetEnabled(false);

        // Detect motion.
        active_slider->GetValueChanged().EnableObserver(this, true);

        // Save the part being dragged for the active controller, if any.
        SetDraggedPart_(true);
    }
    else {
        // Stop tracking motion.
        active_slider->GetValueChanged().EnableObserver(this, false);

        // Transfer the translation from the canvas to the Board.
        const Vector3f motion = root_node_.GetRotation() *
            (root_node_.GetScale() * canvas_->GetTranslation());
        root_node_.SetTranslation(root_node_.GetTranslation() + motion);
        canvas_->SetTranslation(Vector3f::Zero());
        frame_->SetTranslation(Vector3f::Zero());

        // Reset the active slider and turn the other sliders back on.
        active_slider->SetValue(Vector2f::Zero());
        other_slider->SetEnabled(true);
        size_slider_->SetEnabled(is_size_enabled_);

        SetDraggedPart_(false);
    }
}

void Board::Impl_::SizeActivated_(bool is_activation) {
    if (is_activation) {
        // Turn off display of move handles and all size handles that are not
        // being dragged so they do not have to be updated.
        xy_move_slider_->SetEnabled(false);
        xz_move_slider_->SetEnabled(false);

        // Detect size changes.
        size_slider_->GetValueChanged().EnableObserver(this, true);

        // Save the part being dragged for the active controller, if any.
        SetDraggedPart_(true);
    }
    else {
        // Stop tracking size changes.
        size_slider_->GetValueChanged().EnableObserver(this, false);

        // Reset the size slider and turn the move sliders back on.
        size_slider_->SetValue(Vector2f::Zero());
        xy_move_slider_->SetEnabled(true);
        xz_move_slider_->SetEnabled(true);

        // Turn the other handles back on.
        for (auto &child: size_slider_->GetChildren())
            child->SetEnabled(true);

        // Move the handles based on the new size.
        UpdateHandlePositions_();

        SetDraggedPart_(false);
    }
}

void Board::Impl_::Move_(bool is_xy) {
    auto active_slider = is_xy ? xy_move_slider_ : xz_move_slider_;
    const Vector2f &val = active_slider->GetValue();
    const Vector3f offset = is_xy ?
        Vector3f(val[0], val[1], 0) : Vector3f(val[0], 0, val[1]);
    const Vector3f new_pos = start_pos_ + offset;
    canvas_->SetTranslation(new_pos);
    frame_->SetTranslation(new_pos);
}

void Board::Impl_::Size_() {
    // Determine which corner is being dragged and use its translation.
    const auto &info = size_slider_->GetStartDragInfo();
    SG::NodePtr active_handle;
    ASSERT(info.trigger != Trigger::kTouch);
    if (info.trigger == Trigger::kPointer) {
        ASSERT(! info.hit.path.empty());
        active_handle = info.hit.path.back();
    }
    else {  // Grip
        ASSERT(l_grip_state_.is_active || r_grip_state_.is_active);
        active_handle = l_grip_state_.is_active ?
            l_grip_state_.hovered_part : r_grip_state_.hovered_part;
    }
    ASSERT(active_handle);
    const Vector3f offset = active_handle->GetTranslation();

    // Use the size of the segment from whichever corner is being dragged to
    // the center of the canvas to modify the size.
    const Vector2f &val = size_slider_->GetValue();
    const Vector2f new_size = Vector2f(
        std::max(TK::kMinBoardCanvasSize, 2 * std::fabs(offset[0] + val[0])),
        std::max(TK::kMinBoardCanvasSize, 2 * std::fabs(offset[1] + val[1])));

    // Hide the other three handles so they don't need to be updated.
    for (auto &child: size_slider_->GetChildren())
        child->SetEnabled(child == active_handle);

    // Compute the Panel scale based on the distance scale factor.
    const float panel_scale = distance_scale_ * TK::kPanelToWorldScale;

    // Set the new size in world coordinates and convert to Panel coordinates.
    world_size_ = new_size;
    panel_size_ = new_size / panel_scale;

    // Take the current Panel's min size into account.
    auto cur_panel = GetCurrentPanel();
    ASSERT(cur_panel);
    panel_size_ = MaxComponents(cur_panel->GetMinSize(), panel_size_);
    world_size_ = panel_scale * panel_size_;

    // Update the Panel, Canvas, and Frame.
    cur_panel->SetDistanceScale(distance_scale_);
    cur_panel->SetSize(panel_size_);
    UpdateCanvasAndFrame_();
}

void Board::Impl_::UpdateSizeFromPanel_(const Vector2f &panel_size) {
    ASSERT(canvas_);

    // Use the Panel's new size.
    const float panel_scale = distance_scale_ * TK::kPanelToWorldScale;
    panel_size_ = panel_size;
    world_size_ = panel_scale * panel_size_;

    KLOG('p', root_node_.GetDesc() << " setting size to " << panel_size_);

    // Update the placement of the widgets, even if they are disabled.
    UpdateHandlePositions_();

    // Update slider visibility.
    xy_move_slider_->SetEnabled(is_move_enabled_);
    xz_move_slider_->SetEnabled(is_move_enabled_);
    size_slider_->SetEnabled(is_size_enabled_);

    UpdateCanvasAndFrame_();

    if (IsSetUpForTouch_())
        UpdateScaleForTouch_();
}

void Board::Impl_::UpdateCanvasAndFrame_() {
    // Update the size of the canvas and frame.
    canvas_->SetScale(Vector3f(world_size_, 1));
    frame_->FitToSize(world_size_);
}

void Board::Impl_::UpdateScaleForTouch_() {
    // Scale the Board to compensate for being close to the camera. The Board
    // should just about fill the window with an 80 degree FOV. Let D be the
    // Z distance from the camera to the Board.
    //    So  tan(40) = target_size / D
    //        target_size = D * tan(40)
    // Compute y as well and use both.
    const auto canvas_size = .5f * canvas_->GetScaledBounds().GetSize();
    ASSERT(canvas_size[0] > 0 && canvas_size[1] > 0);
    const float target_size =
        TK::kBoardTouchDistance * ion::math::Tangent(Anglef::FromDegrees(40));
    const float scale = TK::kBoardTouchScale *
        target_size / std::max(canvas_size[0], canvas_size[1]);
    root_node_.SetUniformScale(scale);

    // Grip drags are based on world coordinates, so factor in the Board scale
    // when computing relative motion.
    const float grip_scale = scale * TK::kGripDragScale;
    xy_move_slider_->SetGripDragScale(grip_scale);
    xz_move_slider_->SetGripDragScale(grip_scale);
    size_slider_->SetGripDragScale(grip_scale);
}

void Board::Impl_::UpdateHandlePositions_() {
    // Add .5 to move handles off the edges of the board.
    const Vector3f xvec = GetAxis(0, .5f * (1 + world_size_[0]));
    const Vector3f yvec = GetAxis(1, .5f * (1 + world_size_[1]));

    // XY move slider parts.
    handles_[0]->SetTranslation(-xvec);
    handles_[1]->SetTranslation(xvec);
    handles_[2]->SetTranslation(-yvec);
    handles_[3]->SetTranslation(yvec);

    // XZ move slider parts.
    handles_[4]->SetTranslation(-yvec);

    // Size slider parts
    handles_[5]->SetTranslation(-xvec - yvec);
    handles_[6]->SetTranslation( xvec - yvec);
    handles_[7]->SetTranslation(-xvec + yvec);
    handles_[8]->SetTranslation( xvec + yvec);
}

void Board::Impl_::SetDraggedPart_(bool use_hovered_part) {
    if (use_hovered_part) {
        if (l_grip_state_.is_active)
            l_grip_state_.dragged_part = l_grip_state_.hovered_part;
        if (r_grip_state_.is_active)
            r_grip_state_.dragged_part = r_grip_state_.hovered_part;
    }
    else {
        l_grip_state_.dragged_part.reset();
        r_grip_state_.dragged_part.reset();
    }
}

bool Board::Impl_::UpdatePanelGripInfo_(GripInfo &info) {
    // The controller guide has to be close to perpendicular to the Board and
    // the Board's Panel has to be able to do grip hovering.
    auto panel = GetCurrentPanel();
    if (panel && panel->CanGripHover() &&
        AreDirectionsClose(info.guide_direction, -Vector3f::AxisZ(),
                           TK::kMaxPanelGripHoverDirAngle)) {
        // Convert the controller position into Panel coordinates.
        auto rp = Util::CreateTemporarySharedPtr<SG::Node>(&root_node_);
        const SG::CoordConv cc(SG::FindNodePathUnderNode(rp, *panel));
        const Matrix4f w2p = cc.GetRootToObjectMatrix();
        const Point2f panel_pt = ToPoint2f(w2p * info.event.position3D);

        // Ask the Panel for the Widget to hover.
        const auto widget = panel->GetGripWidget(panel_pt);
        if (widget) {
            const auto path_to_widget =
                SG::FindNodePathUnderNode(path_to_root_node_.back(), *widget);
            const auto path =
                SG::NodePath::Stitch(path_to_root_node_, path_to_widget);
            info.widget       = widget;
            info.target_point =
                SG::CoordConv(path).ObjectToRoot(Point3f::Zero());
            return true;
        }
    }
    return false;
}

void Board::Impl_::GetBestGripHoverPart_(const Vector3f &guide_direction,
                                         GripState_ &state) {
    using ion::math::Normalized;

    std::vector<Vector3f> candidates;
    if (is_move_enabled_) {
        candidates.push_back(Vector3f::AxisX());
        candidates.push_back(Vector3f::AxisY());
        candidates.push_back(Vector3f::AxisZ());  // Bar to move in XZ.
    }
    if (is_size_enabled_) {
        const float x = world_size_[0];
        const float y = world_size_[1];
        candidates.push_back(Normalized(Vector3f(x,  y, 0)));  // Bottom left.
        candidates.push_back(Normalized(Vector3f(x, -y, 0)));  // Top left.
    }

    bool is_opposite;
    int index = GetBestDirIndex(candidates, guide_direction, Anglef(),
                                is_opposite);
    ASSERT(index >= 0);

    // Correlate the index to a part.
    int handle_index = -1;
    if (is_move_enabled_) {
        if (index < 3) {
            // Move slider handle.
            switch (index) {
              case 0: handle_index = is_opposite ? 1 : 0; break;  // Left/right.
              case 1: handle_index = is_opposite ? 3 : 2; break;  // Bottom/top.
              case 2: handle_index = 4;                   break;  // XZ Bar.
            }
            state.hovered_slider =
                index < 2 ? xy_move_slider_ : xz_move_slider_;
        }
        else {
            // Size slider handle. Set in code below.
            ASSERT(is_size_enabled_);
            index -= 3;
        }
    }
    if (handle_index < 0) {
        // Size slider handle.
        ASSERT(index == 0 || index == 1);
        if (index == 0)
            handle_index = is_opposite ? 8 : 5;  // BottomLeft/TopRight.
        else
            handle_index = is_opposite ? 6 : 7;  // TopLeft/BottomRight.
        state.hovered_slider = size_slider_;
    }
    ASSERT(handle_index >= 0);
    state.hovered_part = handles_[handle_index];
}

void Board::Impl_::UpdateGripHover_(const GripState_ &state, GripInfo &info) {
    ASSERT(state.hovered_slider);
    info.widget = state.hovered_slider;
    info.color = state.is_active ? GetActiveGripColor() : GetNeutralGripColor();

    // Set the target point based on the active slider.
    Point3f local_pt;
    if (info.widget == xy_move_slider_) {
        // Use the position of the hovered part translated by the canvas
        // position.
        local_pt = Point3f(canvas_->GetTranslation() +
                           state.hovered_part->GetTranslation());
    }
    else if (info.widget == xz_move_slider_) {
        // Use the rotated position of the xz_move_slider_ geometry.
        auto bar  = SG::FindNodeUnderNode(root_node_, "Bar");
        const SG::CoordConv cc(SG::FindNodePathUnderNode(bar, "Crossbar"));
        local_pt = canvas_->GetTranslation() + cc.ObjectToRoot(Point3f::Zero());
    }
    else {
        // Size slider. Need to compute the current position of the dragged
        // handle.
        local_pt = Point3f(state.hovered_part->GetTranslation() +
                           Vector3f(size_slider_->GetValue(), 0));
    }
    ASSERT(! path_to_root_node_.empty());
    info.target_point =
        SG::CoordConv(path_to_root_node_).ObjectToRoot(local_pt);
}

// ----------------------------------------------------------------------------
// Board functions.
// ----------------------------------------------------------------------------

Board::Board() {
}

void Board::AddFields() {
    AddField(behavior_.Init("behavior", Behavior::kReplaces));
    AddField(is_floating_.Init("is_floating", false));
    AddField(touch_offset_.Init("touch_offset", Vector3f::Zero()));

    Grippable::AddFields();
}

void Board::CreationDone() {
    Grippable::CreationDone();
    if (! IsTemplate())
        impl_.reset(new Impl_(*this));
}

void Board::SetPanel(const PanelPtr &panel,
                     const BoardAgent::ResultFunc &result_func) {
    impl_->SetPanel(panel, result_func);
}

void Board::PushPanel(const PanelPtr &panel,
                      const BoardAgent::ResultFunc &result_func) {
    impl_->PushPanel(panel, result_func);
}

bool Board::PopPanel(const Str &result) {
    return impl_->PopPanel(result);
}

size_t Board::GetPanelCount() const {
    return impl_->GetPanelCount();
}

PanelPtr Board::GetCurrentPanel() const {
    return impl_->GetCurrentPanel();
}

void Board::SetDistanceScale(float scale) {
    impl_->SetDistanceScale(scale);
}

void Board::Show(bool shown) {
    impl_->Show(shown);
    SetEnabled(shown);
}

void Board::PostSetUpIon() {
    Grippable::PostSetUpIon();
    impl_->InitCanvas();
}

void Board::UpdateForRenderPass(const Str &pass_name) {
    impl_->UpdateSizeIfNecessary();
    Grippable::UpdateForRenderPass(pass_name);
}

void Board::SetUpForTouch(const Point3f &cam_pos) {
    impl_->SetUpForTouch(cam_pos, GetTouchOffset());
}

void Board::SetOrientation(const Vector3f &view_dir) {
    impl_->SetOrientation(view_dir);
}

void Board::SetPosition(const Point3f &pos) {
    impl_->SetPosition(pos);
}

void Board::SetPath(const SG::NodePath &path) {
    impl_->SetPathToRootNode(path);
}

const SG::Node * Board::GetGrippableNode() const {
    return IsShown() && impl_->IsGrippableEnabled() ? this : nullptr;
}

void Board::UpdateGripInfo(GripInfo &info) {
    impl_->UpdateGripInfo(info);
}

void Board::ActivateGrip(Hand hand, bool is_active) {
    impl_->ActivateGrip(hand, is_active);
}

WidgetPtr Board::GetTouchedWidget(const Point3f &touch_pos,
                                  float radius) const {
    return impl_->GetTouchedWidget(touch_pos, radius);
}

Bounds Board::UpdateBounds() const {
    impl_->UpdateSizeIfNecessary();
    return Grippable::UpdateBounds();
}
