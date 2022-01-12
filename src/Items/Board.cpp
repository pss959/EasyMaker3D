#include "Items/Board.h"

#include "Items/Controller.h"
#include "Items/Frame.h"
#include "Managers/ColorManager.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Widgets/Slider2DWidget.h"

// ----------------------------------------------------------------------------
// Board::Impl_ class.
// ----------------------------------------------------------------------------

class Board::Impl_ {
  public:
    /// The constructor is passed the root node to find all parts under.
    Impl_(SG::Node &root_node) : root_node_(root_node) {}
    void InitCanvas();
    void EnableMove(bool enable);
    void EnableSize(bool enable);
    void SetSize(const Vector2f &size);
    const Vector2f & GetSize() const { return size_; }
    void SetPanel(const PanelPtr &panel);
    const PanelPtr & GetPanel() const { return panel_; }
    void Show(bool shown);
    void UpdateForRenderPass(const std::string &pass_name);
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

    PanelPtr panel_;
    Vector2f size_{0, 0};
    bool is_move_enabled_   = true;
    bool is_size_enabled_ = true;

    Vector3f start_pos_;

    bool may_need_resize_ = true;

    GripState_ l_grip_state_;   ///< Grip state for left controller.
    GripState_ r_grip_state_;   ///< Grip state for right controller.

    /// Finds and stores all of the necessary parts.
    void FindParts_();

    /// Updates all of the parts of the Board for the first time or when
    /// anything changes. Finds them first if necessary.
    void UpdateParts_();

    void UpdateHandlePositions_();
    void MoveActivated_(bool is_activation);
    void SizeActivated_(bool is_activation);
    void Move_();
    void Size_();

    void UpdateSize_(const Vector2f &new_size, bool update_parts);
    void ScaleCanvasAndFrame_();

    void GetBestGripHoverPart_(const Vector3f &guide_direction,
                               GripState_ &state);
};

void Board::Impl_::InitCanvas() {
    // Set the base canvas color.
    if (! canvas_)
        FindParts_();
    canvas_->SetBaseColor(ColorManager::GetSpecialColor("BoardCanvasColor"));
}

void Board::Impl_::EnableMove(bool enable) {
    is_move_enabled_ = enable;
    UpdateParts_();
}

void Board::Impl_::EnableSize(bool enable) {
    is_size_enabled_ = enable;
    UpdateParts_();
}

void Board::Impl_::SetSize(const Vector2f &size) {
    UpdateSize_(size, true);
}

void Board::Impl_::SetPanel(const PanelPtr &panel) {
    ASSERT(panel);

    if (panel_)
        panel_->GetSizeChanged().RemoveObserver(this);

    if (! canvas_)
        FindParts_();

    if (panel_)
        canvas_->RemoveChild(panel_);

    panel_ = panel;
    canvas_->AddChild(panel_);

    // Track changes to the Panel size.
    panel_->GetSizeChanged().AddObserver(
        this, [this](){ may_need_resize_ = true; });

    size_.Set(0, 0);  // Make sure it updates.
    UpdateSize_(size_, true);

    // Ask the Panel whether to show sliders.
    EnableMove(panel->IsMovable());
    EnableSize(panel->IsResizable());
}

void Board::Impl_::Show(bool shown) {
    if (shown) {
        UpdateParts_();
        if (panel_) {
            panel_->SetSize(size_);
            panel_->SetIsShown(true);
        }
    }
    else {
        if (panel_)
            panel_->SetIsShown(false);
    }
}

void Board::Impl_::UpdateForRenderPass(const std::string &pass_name) {
    // If something changed that may affect the size, update.
    if (may_need_resize_) {
        const Vector2f cur_size = size_;
        size_.Set(0, 0);  // Make sure it updates.
        UpdateSize_(cur_size, true);
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
        this, std::bind(&Board::Impl_::MoveActivated_, this,
                        std::placeholders::_2));
    size_slider_->GetActivation().AddObserver(
        this, std::bind(&Board::Impl_::SizeActivated_, this,
                        std::placeholders::_2));
}

void Board::Impl_::UpdateParts_() {
    if (! canvas_)
        FindParts_();

    // Update the size of the canvas and frame.
    ScaleCanvasAndFrame_();

    // Update the placement of the slider widgets, even if they are disabled.
    UpdateHandlePositions_();

    move_slider_->SetEnabled(Flag::kTraversal, is_move_enabled_);
    size_slider_->SetEnabled(Flag::kTraversal, is_size_enabled_);
}

void Board::Impl_::UpdateHandlePositions_() {
    auto set_pos = [this](const std::string &name, const Vector3f &pos){
        SG::FindNodeUnderNode(root_node_, name)->SetTranslation(pos);
    };
    const Vector3f xvec = GetAxis(0, .5f * size_[0]);
    const Vector3f yvec = GetAxis(1, .5f * size_[1]);

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

void Board::Impl_::MoveActivated_(bool is_activation) {
    if (is_activation) {
        // Save the current canvas translation.
        start_pos_ = canvas_->GetTranslation();

        // Turn off display of size handles.
        size_slider_->SetEnabled(Flag::kTraversal, false);

        // Detect motion.
        move_slider_->GetValueChanged().AddObserver(
            this, std::bind(&Board::Impl_::Move_, this));

        // Save the part being dragged for the active controller, if any.
        if (l_grip_state_.is_active)
            l_grip_state_.dragged_part = l_grip_state_.hovered_part;
        if (r_grip_state_.is_active)
            r_grip_state_.dragged_part = r_grip_state_.hovered_part;
    }
    else {
        // Stop tracking motion.
        move_slider_->GetValueChanged().RemoveObserver(this);

        // Transfer the translation from the canvas to the Board.
        root_node_.SetTranslation(
            root_node_.GetTranslation() + canvas_->GetTranslation());
        canvas_->SetTranslation(Vector3f::Zero());
        frame_->SetTranslation(Vector3f::Zero());

        // Reset the move slider and turn the size slider back on.
        move_slider_->SetValue(Vector2f::Zero());
        size_slider_->SetEnabled(Flag::kTraversal, is_size_enabled_);

        l_grip_state_.dragged_part.reset();
        r_grip_state_.dragged_part.reset();
    }
}

void Board::Impl_::SizeActivated_(bool is_activation) {
    if (is_activation) {
        // Turn off display of move handles and all size handles that are not
        // being dragged so they do not have to be updated.
        move_slider_->SetEnabled(Flag::kTraversal, false);

        // Detect size changes.
        size_slider_->GetValueChanged().AddObserver(
            this, std::bind(&Board::Impl_::Size_, this));
    }
    else {
        // Stop tracking size changes.
        size_slider_->GetValueChanged().RemoveObserver(this);

        // Reset the size slider and turn the move slider back on.
        size_slider_->SetValue(Vector2f::Zero());
        move_slider_->SetEnabled(Flag::kTraversal, true);

        // Turn the other handles back on.
        for (auto &child: size_slider_->GetChildren())
            child->SetEnabled(Flag::kTraversal, true);

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
        child->SetEnabled(Flag::kTraversal, child == active_handle);

    // Do not update parts here, since one of them is being dragged.
    UpdateSize_(new_size, false);

    // Do update the size of the canvas and frame.
    ScaleCanvasAndFrame_();
}

void Board::Impl_::UpdateSize_(const Vector2f &new_size, bool update_parts) {
    // Disable the observer to avoid infinite recursion. I hear it's bad.
    if (panel_)
        panel_->GetSizeChanged().EnableObserver(this, false);

    const Vector2f old_size = size_;

    // Respect the panel's minimum size.
    size_ = panel_ ? MaxComponents(panel_->GetMinSize(), new_size) : new_size;

    if (size_ != old_size) {
        if (update_parts && canvas_)
            UpdateParts_();
        if (panel_)
            panel_->SetSize(size_);
    }
    may_need_resize_ = false;

    if (panel_)
        panel_->GetSizeChanged().EnableObserver(this, true);
}

void Board::Impl_::ScaleCanvasAndFrame_() {
    canvas_->SetScale(Vector3f(size_, 1));
    frame_->FitToSize(size_);
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
        const float x = size_[0];
        const float y = size_[1];
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

void Board::EnableMove(bool enable) {
    impl_->EnableMove(enable);
}

void Board::EnableSize(bool enable) {
    impl_->EnableSize(enable);
}

void Board::SetSize(const Vector2f &size) {
    impl_->SetSize(size);
}

const Vector2f & Board::GetSize() const {
    return impl_->GetSize();
}

void Board::SetPanel(const PanelPtr &panel) {
    impl_->SetPanel(panel);
}

const PanelPtr & Board::GetPanel() const {
    return impl_->GetPanel();
}

void Board::Show(bool shown) {
    impl_->Show(shown);
    SetEnabled(Flag::kTraversal, shown);
}

void Board::PostSetUpIon() {
    Grippable::PostSetUpIon();
    impl_->InitCanvas();
}

void Board::UpdateForRenderPass(const std::string &pass_name) {
    impl_->UpdateForRenderPass(pass_name);
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
