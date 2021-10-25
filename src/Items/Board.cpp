#include "Items/Board.h"

#include "Managers/ColorManager.h"
#include "SG/Search.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/Slider2DWidget.h"

// ----------------------------------------------------------------------------
// Board::Parts_ struct.
// ----------------------------------------------------------------------------

/// This struct stores all of the parts the Board needs to operate.
struct Board::Parts_ {
    /// Enum representing a side of the board.
    enum class Side_ { kLeft, kRight, kBottom, kTop };

    /// Enum representing a corner of the board.
    enum class Corner_ { kTopLeft, kTopRight, kBottomLeft, kBottomRight };

    /// Canvas rectangle.
    SG::NodePtr       canvas;

    /// Move sliders.
    Slider1DWidgetPtr move_sliders[4];

    // Resize sliders.
    Slider2DWidgetPtr resize_sliders[4];
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

void Board::EnableMoveHandles(bool enable) {
    are_move_handles_enabled_ = enable;
}

void Board::EnableResizeHandles(bool enable) {
    are_resize_handles_enabled_ = enable;
}

void Board::SetSize(const Vector2f &size) {
    size_ = size;
    UpdateParts_();
}

void Board::Show(bool shown) {
    if (shown)
        UpdateParts_();
    SetEnabled(Flag::kTraversal, shown);
}

void Board::PostSetUpIon() {
    SG::Node::PostSetUpIon();

    // Set the base canvas color.
    if (parts_)
        parts_->canvas->SetBaseColor(
            ColorManager::GetSpecialColor("BoardCanvasColor"));
}

void Board::UpdateParts_() {
    if (! parts_)
        FindParts_();

    // Update the sizes and placement of the parts.
    parts_->canvas->SetScale(Vector3f(size_, 1));
}

void Board::FindParts_() {
    ASSERT(! parts_);
    parts_.reset(new Parts_);

    // Find and create all of the necessary parts.
    parts_->canvas =
        SG::FindNodeUnderNode(*this, "Canvas")->CloneTyped<SG::Node>(false);
    Slider1DWidgetPtr move_slider =
        SG::FindTypedNodeUnderNode<Slider1DWidget>(*this, "MoveSlider");
    Slider2DWidgetPtr resize_slider =
        SG::FindTypedNodeUnderNode<Slider2DWidget>(*this, "ResizeSlider");

    // Set up each move and resize slider.
    for (int i = 0; i < 4; ++i) {
        parts_->move_sliders[i] = move_slider->CloneTyped<Slider1DWidget>(
            false, "MoveSlider" + Util::ToString(i));
        parts_->resize_sliders[i] = resize_slider->CloneTyped<Slider2DWidget>(
            false, "ResizeSlider" + Util::ToString(i));
    }

    // Add all of the visible parts to the Board.
    AddChild(parts_->canvas);
    for (int i = 0; i < 4; ++i)
        AddChild(parts_->move_sliders[i]);
    for (int i = 0; i < 4; ++i)
        AddChild(parts_->resize_sliders[i]);
}
