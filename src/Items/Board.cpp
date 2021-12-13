#include "Items/Board.h"

#include "Items/Frame.h"
#include "Managers/ColorManager.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Widgets/Slider2DWidget.h"

// ----------------------------------------------------------------------------
// Handy constants.
// ----------------------------------------------------------------------------

namespace {

/// Minimum size for either canvas dimension.
static const float kMinCanvasSize_ = 4;

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Board::Parts_ struct.
// ----------------------------------------------------------------------------

/// This struct stores all of the parts the Board needs to operate.
struct Board::Parts_ {
    SG::NodePtr       canvas;       ///< Canvas rectangle.
    Slider2DWidgetPtr move_slider;  ///< Move slider with handles on sides.
    Slider2DWidgetPtr size_slider;  ///< Size slider with handles at corners.
    FramePtr          frame;        ///< Frame around the Board.
};

// ----------------------------------------------------------------------------
// Board functions.
// ----------------------------------------------------------------------------

Board::Board() {
}

void Board::EnableMove(bool enable) {
    is_move_enabled_ = enable;
    UpdateParts_();
}

void Board::EnableSize(bool enable) {
    is_size_enabled_ = enable;
    UpdateParts_();
}

void Board::SetSize(const Vector2f &size) {
    UpdateSize_(size, true);
}

void Board::SetPanel(const PanelPtr &panel) {
    ASSERT(panel);

    if (panel_)
        panel_->GetSizeChanged().RemoveObserver(this);

    if (! parts_)
        FindParts_();

    if (panel_)
        parts_->canvas->RemoveChild(panel_);

    panel_ = panel;
    parts_->canvas->AddChild(panel_);

    // Track changes to the Panel size.
    panel_->GetSizeChanged().AddObserver(
        this, [this](){ may_need_resize_ = true; });

    size_.Set(0, 0);  // Make sure it updates.
    UpdateSize_(size_, true);

    // Ask the Panel whether to show sliders.
    EnableMove(panel->IsMovable());
    EnableSize(panel->IsResizable());
}

void Board::Show(bool shown) {
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
    SetEnabled(Flag::kTraversal, shown);
}

void Board::PostSetUpIon() {
    Grippable::PostSetUpIon();

    // Set the base canvas color.
    if (! parts_)
        FindParts_();
    parts_->canvas->SetBaseColor(
        ColorManager::GetSpecialColor("BoardCanvasColor"));
}

void Board::UpdateForRenderPass(const std::string &pass_name) {
    // If something changed that may affect the size, update.
    if (may_need_resize_) {
        size_.Set(0, 0);  // Make sure it updates.
        UpdateSize_(size_, true);
    }
    Grippable::UpdateForRenderPass(pass_name);
}

void Board::UpdateGripInfo(GripInfo &info) {
    if (grip_dragged_part_) {
        grip_hovered_part_ = grip_dragged_part_;
    }
    else {
        grip_hovered_part_ = GetBestGripHoverPart_(info.event,
                                                   is_size_hovered_);
    }
    info.widget = is_size_hovered_ ? parts_->size_slider : parts_->move_slider;
    info.color  = ColorManager::GetSpecialColor("GripDefaultColor");
    info.target_point = Point3f(grip_hovered_part_->GetTranslation());
}

void Board::FindParts_() {
    ASSERT(! parts_);
    parts_.reset(new Parts_);

    // Find all of the necessary parts.
    parts_->canvas = SG::FindNodeUnderNode(*this, "Canvas");
    parts_->move_slider =
        SG::FindTypedNodeUnderNode<Slider2DWidget>(*this, "MoveSlider");
    parts_->size_slider =
        SG::FindTypedNodeUnderNode<Slider2DWidget>(*this, "SizeSlider");
    parts_->frame = SG::FindTypedNodeUnderNode<Frame>(*this, "BoardFrame");

    // Set up the sliders.
    parts_->move_slider->GetActivation().AddObserver(
        this, std::bind(&Board::MoveActivated_, this, std::placeholders::_2));
    parts_->size_slider->GetActivation().AddObserver(
        this, std::bind(&Board::SizeActivated_, this, std::placeholders::_2));
}

void Board::UpdateParts_() {
    if (! parts_)
        FindParts_();

    // Update the size of the canvas and frame.
    ScaleCanvasAndFrame_();

    // Update the placement of the slider widgets, even if they are disabled.
    UpdateHandlePositions_();

    parts_->move_slider->SetEnabled(Flag::kTraversal, is_move_enabled_);
    parts_->size_slider->SetEnabled(Flag::kTraversal, is_size_enabled_);
}

void Board::UpdateHandlePositions_() {
    auto set_pos = [this](const std::string &name, const Vector3f &pos){
        SG::FindNodeUnderNode(*this, name)->SetTranslation(pos);
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

void Board::MoveActivated_(bool is_activation) {
    if (is_activation) {
        // Save the current canvas translation.
        start_pos_ = parts_->canvas->GetTranslation();

        // Turn off display of size handles.
        parts_->size_slider->SetEnabled(Flag::kTraversal, false);

        // Detect motion.
        parts_->move_slider->GetValueChanged().AddObserver(
            this, std::bind(&Board::Move_, this));

        // Save the part being dragged.
        ASSERT(grip_hovered_part_);
        grip_dragged_part_ = grip_hovered_part_;
    }
    else {
        // Stop tracking motion.
        parts_->move_slider->GetValueChanged().RemoveObserver(this);

        // Transfer the translation from the canvas to the Board.
        SetTranslation(GetTranslation() + parts_->canvas->GetTranslation());
        parts_->canvas->SetTranslation(Vector3f::Zero());
        parts_->frame->SetTranslation(Vector3f::Zero());

        // Reset the move slider and turn the size slider back on.
        parts_->move_slider->SetValue(Vector2f::Zero());
        parts_->size_slider->SetEnabled(Flag::kTraversal, is_size_enabled_);

        grip_dragged_part_.reset();
    }
}

void Board::SizeActivated_(bool is_activation) {
    if (is_activation) {
        // Save the current canvas scale.
        start_scale_ = parts_->canvas->GetScale();

        // Turn off display of move handles and all size handles that are not
        // being dragged so they do not have to be updated.
        parts_->move_slider->SetEnabled(Flag::kTraversal, false);

        // Detect size changes.
        parts_->size_slider->GetValueChanged().AddObserver(
            this, std::bind(&Board::Size_, this));
    }
    else {
        // Stop tracking size changes.
        parts_->size_slider->GetValueChanged().RemoveObserver(this);

        // Reset the size slider and turn the move slider back on.
        parts_->size_slider->SetValue(Vector2f::Zero());
        parts_->move_slider->SetEnabled(Flag::kTraversal, true);

        // Turn the other handles back on.
        for (auto &child: parts_->size_slider->GetChildren())
            child->SetEnabled(Flag::kTraversal, true);

        UpdateHandlePositions_();
    }
}

void Board::Move_() {
    const Vector2f &val = parts_->move_slider->GetValue();
    const Vector3f new_pos = start_pos_ + Vector3f(val, 0);
    parts_->canvas->SetTranslation(new_pos);
    parts_->frame->SetTranslation(new_pos);
}

void Board::Size_() {
    // Determine which corner is being dragged and use its translation.
    const auto &info = parts_->size_slider->GetStartDragInfo();
    SG::NodePtr active_handle;
    if (info.is_grip) {
        active_handle = grip_hovered_part_;
    }
    else {
        const auto &info = parts_->size_slider->GetStartDragInfo();
        ASSERT(! info.path.empty());
        active_handle = info.path.back();
    }
    const Vector3f offset = active_handle->GetTranslation();

    // Use the size of the segment from whichever corner is being dragged to
    // the center of the canvas to modify the size.
    const Vector2f &val = parts_->size_slider->GetValue();
    const Vector2f new_size = Vector2f(
        std::max(kMinCanvasSize_, 2 * std::fabs(offset[0] + val[0])),
        std::max(kMinCanvasSize_, 2 * std::fabs(offset[1] + val[1])));

    // Hide the other three handles so they don't need to be updated.
    for (auto &child: parts_->size_slider->GetChildren())
        child->SetEnabled(Flag::kTraversal, child == active_handle);

    // Do not update parts here, since one of them is being dragged.
    UpdateSize_(new_size, false);

    // Do update the size of the canvas and frame.
    ScaleCanvasAndFrame_();
}

void Board::UpdateSize_(const Vector2f &new_size, bool update_parts) {
    // Disable the observer to avoid infinite recursion. I hear it's bad.
    if (panel_)
        panel_->GetSizeChanged().EnableObserver(this, false);

    const Vector2f old_size = size_;

    // Respect the panel's minimum size.
    size_ = panel_ ? MaxComponents(panel_->GetMinSize(), new_size) : new_size;

    if (size_ != old_size) {  // XXXX
        if (update_parts && parts_)
            UpdateParts_();
        if (panel_)
            panel_->SetSize(size_);
    }
    may_need_resize_ = false;

    if (panel_)
        panel_->GetSizeChanged().EnableObserver(this, true);
}

void Board::ScaleCanvasAndFrame_() {
    parts_->canvas->SetScale(Vector3f(size_, 1));
    parts_->frame->FitToSize(size_);
}

SG::NodePtr Board::GetBestGripHoverPart_(const Event &event, bool &is_size) {
    // Use the orientation of the controller to choose the best handle to
    // hover.
    ASSERT(event.flags.Has(Event::Flag::kOrientation));
    const Vector3f hand_dir = event.device == Event::Device::kLeftController ?
        Vector3f::AxisX() : -Vector3f::AxisX();
    const Vector3f direction = event.orientation * hand_dir;

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

    const size_t index = GetBestDirChoice(choices, direction, Anglef());
    ASSERT(index != ion::base::kInvalidIndex);
    is_size = index >= first_size_index;
    return SG::FindNodeUnderNode(*this, choices[index].name);
}
