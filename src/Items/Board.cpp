#include "Items/Board.h"

#include "Defaults.h"
#include "Items/Controller.h"
#include "Items/Frame.h"
#include "Managers/ColorManager.h"
#include "Panels/Panel.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Widgets/Slider2DWidget.h"

// ----------------------------------------------------------------------------
// Board::Impl_ class.
// ----------------------------------------------------------------------------

class Board::Impl_ {
  public:
    /// The constructor is passed the root node to find all parts under.
    Impl_(SG::Node &root_node) : root_node_(root_node) {}
    void InitCanvas();
    void EnableMoveAndSize(bool enable_move, bool enable_size);
    void SetPanel(const PanelPtr &panel);
    void SetPanelScale(float scale);
    const PanelPtr & GetPanel() const { return panel_; }
    void Show(bool shown);
    void UpdateSizeIfNecessary();
    bool IsGrippableEnabled() const {
        return is_move_enabled_ || is_size_enabled_;
    }
    void UpdateGripInfo(GripInfo &info);
    void ActivateGrip(Hand hand, bool is_active);

  private:
    /// Minimum size for either canvas dimension.
    static constexpr float kMinCanvasSize_ = 4;

    /// This struct represents the current grip state for a controller.
    struct GripState_ {
        /// This saves the part that was last hovered in UpdateGripInfo() so
        /// that the Size_() function knows what to do.
        SG::NodePtr hovered_part;

        /// Set to true if grip_hovered_part_ is in the size slider as opposed
        /// to the move slider.
        bool is_size_hovered = false;

        /// This saves the part that is currently being dragged so that it is
        /// the only one that is hovered.
        SG::NodePtr dragged_part;

        /// True if the grip button is active.
        bool is_active = false;
    };

    SG::Node &root_node_;

    // Parts.
    SG::NodePtr       canvas_;       ///< Canvas rectangle.
    Slider2DWidgetPtr move_slider_;  ///< Move slider with handles on sides.
    Slider2DWidgetPtr size_slider_;  ///< Size slider with handles at corners.
    FramePtr          frame_;        ///< Frame around the Board.

    // Sizes.
    Vector2f          world_size_{0, 0};  ///< Board size in world coordinates.
    Vector2f          panel_size_{0, 0};  ///< Board size in panel coordinates.

    PanelPtr          panel_;
    float             panel_scale_ = Defaults::kPanelToWorld;
    bool              is_move_enabled_ = true;
    bool              is_size_enabled_ = true;
    Vector3f          start_pos_;               ///< Used for computing motion.

    // Grip state.
    GripState_        l_grip_state_;   ///< Grip state for left controller.
    GripState_        r_grip_state_;   ///< Grip state for right controller.

    /// Finds and stores all of the necessary parts.
    void FindParts_();

    // Move and size slider callbacks.
    void MoveActivated_(bool is_activation);  ///< Move slider de/activation.
    void SizeActivated_(bool is_activation);  ///< Size slider de/activation.
    void Move_();                             ///< Move slider change callback.
    void Size_();                             ///< Size slider change callback.

    /// Updates the Board in response to a size change in the Panel.
    void UpdateSizeFromPanel_();

    /// Updates the sizes of the canvas and the Frame.
    void UpdateCanvasAndFrame_();

    /// Updates the positions of handles based on world_size_.
    void UpdateHandlePositions_();

    /// Returns the best part to grip hover based on the controller direction.
    void GetBestGripHoverPart_(const Vector3f &guide_direction,
                               GripState_ &state);
};

void Board::Impl_::InitCanvas() {
    // Set the base canvas color.
    if (! canvas_)
        FindParts_();
    canvas_->SetBaseColor(ColorManager::GetSpecialColor("BoardCanvasColor"));
}

void Board::Impl_::EnableMoveAndSize(bool enable_move, bool enable_size) {
    is_move_enabled_ = enable_move;
    is_size_enabled_ = enable_size;
}

void Board::Impl_::SetPanel(const PanelPtr &panel) {
    ASSERT(panel);

    if (! canvas_)
        FindParts_();

    if (panel_)
        canvas_->RemoveChild(panel_);

    panel_ = panel;
    canvas_->AddChild(panel_);

    // Ask the Panel whether to show sliders.
    EnableMoveAndSize(panel->IsMovable(), panel->IsResizable());

    // If the Panel size was never set, set it now. Otherwise, update the Board
    // to the Panel's current size.
    if (panel_->GetSize() == Vector2f::Zero()) {
        panel_->SetSize(panel_->GetMinSize());
    }
    else {
        panel_size_ = panel_->GetSize();
        world_size_ = panel_scale_ * panel_size_;
        UpdateCanvasAndFrame_();
    }
}

void Board::Impl_::SetPanelScale(float scale) {
    panel_scale_ = scale;
}

void Board::Impl_::Show(bool shown) {
    if (panel_)
        panel_->SetIsShown(shown);
}

void Board::Impl_::UpdateSizeIfNecessary() {
    if (panel_ && panel_->SizeMayHaveChanged()) {
        // Make sure the Panel has the correct size and update the Board size
        // to match if necessary.
        KLOG('q', "Panel size change detected for Board");
        if (panel_->UpdateSize() != panel_size_)
            UpdateSizeFromPanel_();
    }
}

void Board::Impl_::UpdateGripInfo(GripInfo &info) {
    auto &state = info.event.device == Event::Device::kLeftController ?
        l_grip_state_ : r_grip_state_;

    // If in the middle of a drag, hover the dragged part.
    if (state.dragged_part) {
        state.hovered_part = state.dragged_part;
    }
    // Otherwise, use the controller orientation to get the best part to hover.
    else {
        GetBestGripHoverPart_(info.guide_direction, state);
    }
    info.widget = state.is_size_hovered ? size_slider_ : move_slider_;
    info.color  = ColorManager::GetSpecialColor("GripDefaultColor");
    info.target_point = Point3f(state.hovered_part->GetTranslation());
}

void Board::Impl_::ActivateGrip(Hand hand, bool is_active) {
    if (hand == Hand::kLeft)
        l_grip_state_.is_active = is_active;
    else
        r_grip_state_.is_active = is_active;
}

void Board::Impl_::FindParts_() {
    ASSERT(! canvas_);

    // Find all of the necessary parts.
    canvas_ = SG::FindNodeUnderNode(root_node_, "Canvas");
    move_slider_ =
        SG::FindTypedNodeUnderNode<Slider2DWidget>(root_node_, "MoveSlider");
    size_slider_ =
        SG::FindTypedNodeUnderNode<Slider2DWidget>(root_node_, "SizeSlider");
    frame_ = SG::FindTypedNodeUnderNode<Frame>(root_node_, "BoardFrame");

    // Set up the sliders.
    move_slider_->GetActivation().AddObserver(
        this, [&](Widget &, bool is_act){ MoveActivated_(is_act); });
    move_slider_->GetValueChanged().AddObserver(
        this, [&](Widget &, const Vector2f &){ Move_(); });
    size_slider_->GetActivation().AddObserver(
        this, [&](Widget &, bool is_act){ SizeActivated_(is_act); });
    size_slider_->GetValueChanged().AddObserver(
        this, [&](Widget &, const Vector2f &){ Size_(); });

    // Don't track motion until activation.
    move_slider_->GetValueChanged().EnableObserver(this, true);
    size_slider_->GetValueChanged().EnableObserver(this, true);
}

void Board::Impl_::MoveActivated_(bool is_activation) {
    if (is_activation) {
        // Save the current canvas translation.
        start_pos_ = canvas_->GetTranslation();

        // Turn off display of size handles.
        size_slider_->SetEnabled(false);

        // Detect motion.
        move_slider_->GetValueChanged().EnableObserver(this, true);

        // Save the part being dragged for the active controller, if any.
        if (l_grip_state_.is_active)
            l_grip_state_.dragged_part = l_grip_state_.hovered_part;
        if (r_grip_state_.is_active)
            r_grip_state_.dragged_part = r_grip_state_.hovered_part;
    }
    else {
        // Stop tracking motion.
        move_slider_->GetValueChanged().EnableObserver(this, false);

        // Transfer the translation from the canvas to the Board.
        root_node_.SetTranslation(
            root_node_.GetTranslation() + canvas_->GetTranslation());
        canvas_->SetTranslation(Vector3f::Zero());
        frame_->SetTranslation(Vector3f::Zero());

        // Reset the move slider and turn the size slider back on.
        move_slider_->SetValue(Vector2f::Zero());
        size_slider_->SetEnabled(is_size_enabled_);

        l_grip_state_.dragged_part.reset();
        r_grip_state_.dragged_part.reset();
    }
}

void Board::Impl_::SizeActivated_(bool is_activation) {
    if (is_activation) {
        // Turn off display of move handles and all size handles that are not
        // being dragged so they do not have to be updated.
        move_slider_->SetEnabled(false);

        // Detect size changes.
        size_slider_->GetValueChanged().EnableObserver(this, true);
    }
    else {
        // Stop tracking size changes.
        size_slider_->GetValueChanged().EnableObserver(this, false);

        // Reset the size slider and turn the move slider back on.
        size_slider_->SetValue(Vector2f::Zero());
        move_slider_->SetEnabled(true);

        // Turn the other handles back on.
        for (auto &child: size_slider_->GetChildren())
            child->SetEnabled(true);

        // Move the handles based on the new size.
        UpdateHandlePositions_();
    }
}

void Board::Impl_::Move_() {
    const Vector2f &val = move_slider_->GetValue();
    const Vector3f new_pos = start_pos_ + Vector3f(val, 0);
    canvas_->SetTranslation(new_pos);
    frame_->SetTranslation(new_pos);
}

void Board::Impl_::Size_() {
    // Determine which corner is being dragged and use its translation.
    const auto &info = size_slider_->GetStartDragInfo();
    SG::NodePtr active_handle;
    if (info.is_grip) {
        ASSERT(l_grip_state_.is_active || r_grip_state_.is_active);
        active_handle = l_grip_state_.is_active ?
            l_grip_state_.hovered_part : r_grip_state_.hovered_part;
    }
    else {
        ASSERT(! info.hit.path.empty());
        active_handle = info.hit.path.back();
    }
    const Vector3f offset = active_handle->GetTranslation();

    // Use the size of the segment from whichever corner is being dragged to
    // the center of the canvas to modify the size.
    const Vector2f &val = size_slider_->GetValue();
    const Vector2f new_size = Vector2f(
        std::max(kMinCanvasSize_, 2 * std::fabs(offset[0] + val[0])),
        std::max(kMinCanvasSize_, 2 * std::fabs(offset[1] + val[1])));

    // Hide the other three handles so they don't need to be updated.
    for (auto &child: size_slider_->GetChildren())
        child->SetEnabled(child == active_handle);

    // Set the new size in world coordinates and convert to Panel coordinates.
    world_size_ = new_size;
    panel_size_ = new_size / panel_scale_;

    // Take the Panel's min size into account.
    panel_size_ = MaxComponents(panel_->GetMinSize(), panel_size_);
    world_size_ = panel_scale_ * panel_size_;

    // Update the Panel, Canvas, and Frame.
    panel_->SetSize(panel_size_);
    UpdateCanvasAndFrame_();
}

void Board::Impl_::UpdateSizeFromPanel_() {
    ASSERT(canvas_);
    ASSERT(panel_);

    KLOG('p', "Board setting size to " << panel_->GetSize());

    // Use the Panel's new size.
    panel_size_ = panel_->GetSize();
    world_size_ = panel_scale_ * panel_size_;

    // Update the placement of the widgets, even if they are disabled.
    UpdateHandlePositions_();

    // Update slider visibility.
    move_slider_->SetEnabled(is_move_enabled_);
    size_slider_->SetEnabled(is_size_enabled_);

    UpdateCanvasAndFrame_();
}

void Board::Impl_::UpdateCanvasAndFrame_() {
    // Update the size of the canvas and frame.
    canvas_->SetScale(Vector3f(world_size_, panel_scale_));
    frame_->FitToSize(world_size_);
}

void Board::Impl_::UpdateHandlePositions_() {
    // Add .5 to move handles off the edges of the board.
    const Vector3f xvec = GetAxis(0, .5f * (1 + world_size_[0]));
    const Vector3f yvec = GetAxis(1, .5f * (1 + world_size_[1]));

    auto set_pos = [this](const std::string &name, const Vector3f &pos){
        SG::FindNodeUnderNode(root_node_, name)->SetTranslation(pos);
    };

    // Move slider parts.
    set_pos("Left",   -xvec);
    set_pos("Right",   xvec);
    set_pos("Bottom", -yvec);
    set_pos("Top",     yvec);

    // Size slider parts
    set_pos("BottomLeft",  -xvec - yvec);
    set_pos("BottomRight",  xvec - yvec);
    set_pos("TopLeft",     -xvec + yvec);
    set_pos("TopRight",     xvec + yvec);
}

void Board::Impl_::GetBestGripHoverPart_(const Vector3f &guide_direction,
                                         GripState_ &state) {
    std::vector<DirChoice> choices;
    if (is_move_enabled_) {
        choices.push_back(DirChoice("Left",    Vector3f::AxisX()));
        choices.push_back(DirChoice("Right",  -Vector3f::AxisX()));
        choices.push_back(DirChoice("Bottom",  Vector3f::AxisY()));
        choices.push_back(DirChoice("Top",    -Vector3f::AxisY()));
    }
    const size_t first_size_index = choices.size();
    if (is_size_enabled_) {
        const float x = world_size_[0];
        const float y = world_size_[1];
        choices.push_back(DirChoice("BottomLeft",  Vector3f( x,  y, 0)));
        choices.push_back(DirChoice("BottomRight", Vector3f(-x,  y, 0)));
        choices.push_back(DirChoice("TopLeft",     Vector3f( x, -y, 0)));
        choices.push_back(DirChoice("TopRight",    Vector3f(-x, -y, 0)));
    }

    const size_t index = GetBestDirChoice(choices, guide_direction, Anglef());
    ASSERT(index != ion::base::kInvalidIndex);
    state.hovered_part = SG::FindNodeUnderNode(root_node_, choices[index].name);
    state.is_size_hovered = index >= first_size_index;
}

// ----------------------------------------------------------------------------
// Board functions.
// ----------------------------------------------------------------------------

Board::Board() : impl_(new Impl_(*this)) {
}

void Board::SetPanel(const PanelPtr &panel) {
    impl_->SetPanel(panel);
}

void Board::SetPanelScale(float scale) {
    impl_->SetPanelScale(scale);
}

const PanelPtr & Board::GetPanel() const {
    return impl_->GetPanel();
}

void Board::Show(bool shown) {
    impl_->Show(shown);
    SetEnabled(shown);
}

void Board::PostSetUpIon() {
    Grippable::PostSetUpIon();
    impl_->InitCanvas();
}

void Board::UpdateForRenderPass(const std::string &pass_name) {
    impl_->UpdateSizeIfNecessary();
    Grippable::UpdateForRenderPass(pass_name);
}

bool Board::IsGrippableEnabled() const {
    return IsShown() && impl_->IsGrippableEnabled();
}

void Board::UpdateGripInfo(GripInfo &info) {
    impl_->UpdateGripInfo(info);
}

void Board::ActivateGrip(Hand hand, bool is_active) {
    impl_->ActivateGrip(hand, is_active);
}

Bounds Board::UpdateBounds() const {
    impl_->UpdateSizeIfNecessary();
    return Grippable::UpdateBounds();
}
