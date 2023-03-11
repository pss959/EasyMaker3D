#include "Tools/ColorTool.h"

#include "Base/Procedural.h"
#include "Managers/CommandManager.h"
#include "Math/ColorRing.h"
#include "Math/Curves.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "Models/Model.h"
#include "Place/ClickInfo.h"
#include "SG/Node.h"
#include "SG/PolyLine.h"
#include "SG/ProceduralImage.h"
#include "SG/Search.h"
#include "SG/Texture.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/Tuning.h"

ColorTool::ColorTool() {
}

void ColorTool::CreationDone() {
    FloatingTool::CreationDone();

    if (! IsTemplate())
        FindParts_();
}

void ColorTool::UpdateGripInfo(GripInfo &info) {
    info.target_point = ToWorld(marker_, Point3f::Zero());
    info.widget       = widget_;
    info.color        = GetModelAttachedTo()->GetColor();
}

void ColorTool::Attach() {
    ASSERT(widget_);
    UpdateColor_();

    // Position the ColorTool above the center of the attached Model.
    const float height = GetScaledBounds().GetSize()[1];
    SetTranslation(GetPositionAboveModel(.5f * height + 2, false));
}

void ColorTool::Detach() {
    // Nothing to do here.
}

void ColorTool::FindParts_() {
    ASSERT(! widget_);
    widget_ = SG::FindTypedNodeUnderNode<GenericWidget>(*this, "Widget");
    marker_ = SG::FindNodeUnderNode(*this, "Marker");
    disc_   = SG::FindNodeUnderNode(*this, "Disc");

    // Set up the marker lines to form a circle.
    const float kMarkerRadius = .06f;
    std::vector<Point3f> points = Util::ConvertVector<Point3f, Point2f>(
        GetCirclePoints(20, kMarkerRadius, true),
        [](const Point2f &p){ return Point3f(p, 0); });
    points.push_back(points[0]);  // Closes the circle.
    auto line = SG::FindTypedShapeInNode<SG::PolyLine>(*marker_, "Line");
    line->SetPoints(points);

    widget_->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ Clicked_(info); });
    widget_->GetDragged().AddObserver(
        this,
        [&](const DragInfo *info, bool is_start){ Dragged_(info, is_start); });

    // Set up the ProceduralImage to render the color ring.
    auto ring = SG::FindNodeUnderNode(*this, "Ring");
    ASSERT(! ring->GetUniformBlocks().empty());
    const auto &block = ring->GetUniformBlocks()[0];
    ASSERT(block);
    ASSERT(! block->GetTextures().empty());
    const auto &tex = block->GetTextures()[0];
    ASSERT(tex);
    const auto &proc_im =
        Util::CastToDerived<SG::ProceduralImage>(tex->GetImage());
    ASSERT(proc_im);
    proc_im->SetFunction([](){ return GenerateColorRingImage(); });
}

void ColorTool::Clicked_(const ClickInfo &info) {
    ASSERT(! command_);
    command_ = CreateCommand<ChangeColorCommand>();
    command_->SetFromSelection(GetSelection());
    command_->SetNewColor(GetRingColor_(info.hit.point));
    GetContext().command_manager->AddAndDo(command_);
    command_.reset();
    UpdateColor_();
}

void ColorTool::Dragged_(const DragInfo *info, bool is_start) {
    // Note that is_start is true for the start of a drag and info is null for
    // the end of a drag.
    if (is_start) {
        ASSERT(! command_);
        command_ = CreateCommand<ChangeColorCommand>();
        command_->SetFromSelection(GetSelection());
        GetDragStarted().Notify(*this);
        start_ring_pt_ = Point3f(marker_->GetTranslation());
    }
    else if (info) {
        // Middle of the drag: simulate execution of the command to update all
        // the Models.
        bool    got_pos = true;
        Point3f ring_pt;
        if (info->trigger == Trigger::kPointer) {
            if (info->hit.path.ContainsNode(*widget_))
                ring_pt = info->hit.point;
            else
                got_pos = false;
        }
        else if (info->trigger == Trigger::kGrip) {
            // Use a point relative to the grip starting point.
            const auto &p0 = widget_->GetStartDragInfo().grip_position;
            const auto &p1 = info->grip_position;
            ring_pt = start_ring_pt_ + TK::kColorToolGripDragScale * (p1 - p0);
        }
        ASSERT(command_);
        if (got_pos) {
            command_->SetNewColor(GetRingColor_(ring_pt));
            GetContext().command_manager->SimulateDo(command_);
        }
    }
    else {
        // End of the drag.
        GetDragEnded().Notify(*this);
        ASSERT(command_);
        // Don't apply the command if the color was never set.
        if (command_->GetNewColor() != Color::Black())
            GetContext().command_manager->AddAndDo(command_);
        command_.reset();
    }
    UpdateColor_();
}

void ColorTool::UpdateColor_() {
    ASSERT(GetModelAttachedTo());
    const Color &color = GetModelAttachedTo()->GetColor();

    // Move the marker to the correct spot. Put it in front a little bit.
    const Point2f marker_pt = ColorRing::GetPointForColor(color);
    marker_->SetTranslation(Point3f(marker_pt, .1f));

    // Change the color of the central disc.
    disc_->SetBaseColor(color);
}

Color ColorTool::GetRingColor_(const Point3f &ring_point) {
    return ColorRing::GetColorForPoint(ToPoint2f(ring_point));
}
