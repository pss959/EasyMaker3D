#include "Items/Board.h"

#include "Managers/ColorManager.h"
#include "SG/Search.h"
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

    parts_->move_slider->SetEnabled(Flag::kTraversal, is_move_enabled_);
    parts_->size_slider->SetEnabled(Flag::kTraversal, is_size_enabled_);
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

void Board::MoveActivated_(bool is_activation) {
    if (is_activation) {
        // Turn off display of size handles.
        parts_->size_slider->SetEnabled(Flag::kTraversal, false);

        // Detect motion.
        parts_->move_slider->GetValueChanged().AddObserver(
            this, std::bind(&Board::Move_, this));
    }
    else {
        parts_->move_slider->GetValueChanged().RemoveObserver(this);
        parts_->move_slider->SetValue(Vector2f::Zero());
        parts_->size_slider->SetEnabled(Flag::kTraversal, true);
    }
}

void Board::SizeActivated_(bool is_activation) {
    if (is_activation) {
        // Turn off display of move handles.
        parts_->move_slider->SetEnabled(Flag::kTraversal, false);

        // Detect motion.
        parts_->size_slider->GetValueChanged().AddObserver(
            this, std::bind(&Board::Size_, this));
    }
    else {
        parts_->size_slider->GetValueChanged().RemoveObserver(this);
        parts_->size_slider->SetValue(Vector2f::Zero());
        parts_->move_slider->SetEnabled(Flag::kTraversal, true);
    }
}

void Board::Move_() {
    std::cerr << "XXXX Move\n";
}

void Board::Size_() {
    std::cerr << "XXXX Size\n";
}
