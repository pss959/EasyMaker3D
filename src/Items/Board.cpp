#include "Items/Board.h"

#include "Managers/ColorManager.h"
#include "SG/Search.h"
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
};

// ----------------------------------------------------------------------------
// Board functions.
// ----------------------------------------------------------------------------

Board::Board() {
}

void Board::AddFields() {
    // XXXX
    SG::Node::AddFields();
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
    size_ = size;
    UpdateParts_();
    if (pane_)
        pane_->SetSize(size);
}

void Board::SetPane(const PanePtr &pane) {
    if (! parts_)
        FindParts_();

    if (pane_)
        parts_->canvas->RemoveChild(pane_);
    pane_ = pane;
    pane_->SetSize(size_);
    parts_->canvas->AddChild(pane_);
}

void Board::Show(bool shown) {
    if (shown) {
        UpdateParts_();
        if (pane_)
            pane_->SetSize(size_);
    }
    SetEnabled(Flag::kTraversal, shown);
}

void Board::PostSetUpIon() {
    SG::Node::PostSetUpIon();

    // Set the base canvas color.
    if (! parts_)
        FindParts_();
    parts_->canvas->SetBaseColor(
        ColorManager::GetSpecialColor("BoardCanvasColor"));
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

    // Set up the sliders.
    parts_->move_slider->GetActivation().AddObserver(
        this, std::bind(&Board::MoveActivated_, this, std::placeholders::_2));
    parts_->size_slider->GetActivation().AddObserver(
        this, std::bind(&Board::SizeActivated_, this, std::placeholders::_2));
}

void Board::UpdateParts_() {
    if (! parts_)
        FindParts_();

    // Update the size of the canvas.
    parts_->canvas->SetScale(Vector3f(size_, 1));

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
    }
    else {
        // Stop tracking motion.
        parts_->move_slider->GetValueChanged().RemoveObserver(this);

        // Transfer the translation from the canvas to the Board.
        SetTranslation(GetTranslation() + parts_->canvas->GetTranslation());
        parts_->canvas->SetTranslation(Vector3f::Zero());

        // Reset the move slider and turn the size slider back on.
        parts_->move_slider->SetValue(Vector2f::Zero());
        parts_->size_slider->SetEnabled(Flag::kTraversal, true);
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
    parts_->canvas->SetTranslation(start_pos_ + Vector3f(val, 0));
}

void Board::Size_() {
    // Determine which corner is being dragged and use its translation.
    const auto &info = parts_->size_slider->GetStartDragInfo();
    ASSERT(! info.hit.path.empty());
    const auto &active_handle = info.hit.path.back();
    const Vector3f offset = active_handle->GetTranslation();

    // Use the size of the segment from whichever corner is being dragged to
    // the center of the canvas to modify the size.
    const Vector2f &val = parts_->size_slider->GetValue();
    size_ = Vector2f(
        std::max(kMinCanvasSize_, 2 * std::fabs(offset[0] + val[0])),
        std::max(kMinCanvasSize_, 2 * std::fabs(offset[1] + val[1])));
    parts_->canvas->SetScale(Vector3f(size_, 1));

    // Hide the other three handles so they don't need to be updated.
    for (auto &child: parts_->size_slider->GetChildren())
        child->SetEnabled(Flag::kTraversal, child == active_handle);

    // Update the Pane.
    if (pane_)
        pane_->SetSize(size_);
}
