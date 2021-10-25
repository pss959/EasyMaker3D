#include "Items/Board.h"

#include "Managers/ColorManager.h"
#include "SG/Search.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/Slider2DWidget.h"

// ----------------------------------------------------------------------------
// Handy constants.
// ----------------------------------------------------------------------------

namespace {

// XXXX

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Board::Parts_ struct.
// ----------------------------------------------------------------------------

/// This struct stores all of the parts the Board needs to operate.
struct Board::Parts_ {
    /// Canvas rectangle.
    SG::NodePtr       canvas;

    /// Move sliders, 1 per Side_.
    Slider1DWidgetPtr move_sliders[4];

    // Resize sliders, 1 per Corner_.
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
    UpdateParts_();
}

void Board::EnableResizeHandles(bool enable) {
    are_resize_handles_enabled_ = enable;
    UpdateParts_();
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

    // Update the size of the canvas.
    parts_->canvas->SetScale(Vector3f(size_, 1));

    // Update the placement of the slider widgets, even if they are disabled.
    const Vector3f xvec = GetAxis(0, .5f * size_[0]);
    const Vector3f yvec = GetAxis(1, .5f * size_[1]);

    auto init_move_slider = [this](Side_ side, const Vector3f &pos){
        const auto &ms = parts_->move_sliders[Util::EnumInt(side)];
        ms->SetTranslation(pos);
        ms->SetEnabled(Flag::kTraversal, are_move_handles_enabled_);
    };

    auto init_resize_slider = [this](Corner_ corner, const Vector3f &pos){
        const auto &rs = parts_->resize_sliders[Util::EnumInt(corner)];
        rs->SetTranslation(pos);
        rs->SetEnabled(Flag::kTraversal, are_resize_handles_enabled_);
    };

    init_move_slider(Side_::kLeft,   -xvec);
    init_move_slider(Side_::kRight,   xvec);
    init_move_slider(Side_::kBottom, -yvec);
    init_move_slider(Side_::kTop,     yvec);

    init_resize_slider(Corner_::kBottomLeft,  -xvec - yvec);
    init_resize_slider(Corner_::kBottomRight,  xvec - yvec);
    init_resize_slider(Corner_::kTopLeft,     -xvec + yvec);
    init_resize_slider(Corner_::kTopRight,     xvec + yvec);
}

void Board::FindParts_() {
    ASSERT(! parts_);
    parts_.reset(new Parts_);

    // Find all of the necessary parts.
    auto canvas = SG::FindNodeUnderNode(*this, "Canvas");
    auto move_slider =
        SG::FindTypedNodeUnderNode<Slider1DWidget>(*this, "MoveSlider");
    auto resize_slider =
        SG::FindTypedNodeUnderNode<Slider2DWidget>(*this, "ResizeSlider");

    // Clone so that everything is unique to this Board instance.
    parts_->canvas = canvas->CloneTyped<SG::Node>(false);

    // Set up each move slider.
    for (auto side: Util::EnumValues<Side_>()) {
        const std::string name = "MoveSlider_" + Util::EnumName(side);
        auto ms = move_slider->CloneTyped<Slider1DWidget>(true, name);
        if (side == Side_::kBottom || side == Side_::kTop)
            ms->SetDimension(1);
        ms->GetActivation().AddObserver(
            this, std::bind(&Board::MoveSliderActivated_, this, side,
                            std::placeholders::_2));
        parts_->move_sliders[Util::EnumInt(side)] = ms;
    }

    // Set up each resize slider.
    for (auto corner: Util::EnumValues<Corner_>()) {
        const std::string name = "ResizeSlider_" + Util::EnumName(corner);
        auto rs = resize_slider->CloneTyped<Slider2DWidget>(true, name);
        parts_->resize_sliders[Util::EnumInt(corner)] = rs;
    }

    // Add all of the visible parts to the Board.
    AddChild(parts_->canvas);
    for (int i = 0; i < 4; ++i)
        AddChild(parts_->move_sliders[i]);
    for (int i = 0; i < 4; ++i)
        AddChild(parts_->resize_sliders[i]);
}

void Board::MoveSliderActivated_(Side_ side, bool is_activation) {
    std::cerr << "XXXX Side = " << Util::EnumName(side)
              << " is_activation = " << is_activation << "\n";
}
