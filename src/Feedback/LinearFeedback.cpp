#include "Feedback/LinearFeedback.h"

#include <algorithm>
#include <cmath>

#include <ion/math/vectorutils.h>

#include "SG/Line.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// LinearFeedback::Frame_ struct.
// ----------------------------------------------------------------------------

//! This struct stores everything needed to draw linear feedback.
struct LinearFeedback::Frame_ {
    /// \name Inputs.
    ///@{
    Point3f  p0;               ///< Starting point of feedback.
    Vector3f direction;        ///< Linear feedback direction.
    float    length;           ///< Length of feedback.
    ///@}

    /// \name Computed values.
    ///@{
    Point3f  p1;               ///< Ending point of feedback.
    Vector3f up_direction;     ///< Direction for uprights.
    float    crossbar_height;  ///< Height of crossbar above points.
    float    upright_length;   ///< Length of uprights.
    float    text_height;      ///< Height of text above points.
    ///@}
};

// ----------------------------------------------------------------------------
// LinearFeedback::Parts_ struct.
// ----------------------------------------------------------------------------

/// This struct stores all of the parts the LinearFeedback needs to operate.
struct LinearFeedback::Parts_ {
    SG::LinePtr     upright0;   ///< Upright connected to p0.
    SG::LinePtr     upright1;   ///< Upright connected to p1.
    SG::LinePtr     crossbar;   ///< Crossbar connecting the uprights.
    SG::TextNodePtr text;       ///< TextNode showing the feedback value.
};

// ----------------------------------------------------------------------------
// Various constants used to adjust the look of the feedback.
// ----------------------------------------------------------------------------

static const float kExtraHeight_      = .5f;
static const float kLineWidth_        = .1f;  // XXXX
static const float kExtensionLength_  = 2.f;
static const float kHeightMult_       = 1.1f;
static const float kTextHeightOffset_ = 1.f;
static const float kMinTextY_         = 1.f;

// ----------------------------------------------------------------------------
// LinearFeedback functions.
// ----------------------------------------------------------------------------

LinearFeedback::LinearFeedback() {
}

void LinearFeedback::SetColor(const Color &color) {
    color_ = color;
}

void LinearFeedback::SpanPoints(const Point3f &p0, const Point3f &p1) {
    const float length = ion::math::Distance(p0, p1);
    ASSERT(length > 0);
    SpanLength(p0, ion::math::Normalized(p1 - p0), length);
}

void LinearFeedback::SpanLength(const Point3f &pt, const Vector3f &dir,
                                float length) {
    if (! parts_)
        FindParts_();

    Frame_ frame;
    ComputeFrame_(pt, dir, length, frame);
    const Vector3f upright_vec = frame.upright_length * frame.up_direction;

    // Update the upright lines.
    parts_->upright0->SetEndpoints(frame.p0, frame.p0 + upright_vec);
    parts_->upright1->SetEndpoints(frame.p1, frame.p1 + upright_vec);

    // Update the crossbar line.
    const Vector3f crossbar_up = frame.crossbar_height * frame.up_direction;
    parts_->crossbar->SetEndpoints(frame.p0 + crossbar_up,
                                   frame.p1 + crossbar_up);

    // Update the text. Do NOT let it text go below the minimum.
    Point3f text_pos = frame.p1 + frame.text_height * frame.up_direction;
    text_pos[1] = std::max(text_pos[1], kMinTextY_);
    parts_->text->SetTranslation(Vector3f(text_pos));
    // XXXX Make the text face the camera.
    parts_->text->SetTextWithColor(
        Util::ToString(std::roundf(100 * frame.length) / 100), color_);

    // Set the color for all other parts.
    SetBaseColor(color_);
}

void LinearFeedback::FindParts_() {
    ASSERT(! parts_);
    parts_.reset(new Parts_);

    auto get_line_func = [this](const std::string &name){
        SG::NodePtr n = SG::FindNodeUnderNode(*this, name);
        ASSERT(n->GetShapes().size() == 1U);
        SG::LinePtr line =  Util::CastToDerived<SG::Line>(n->GetShapes()[0]);
        ASSERT(line);
        return line;
    };

    parts_->upright0 = get_line_func("Upright0");
    parts_->upright1 = get_line_func("Upright1");
    parts_->crossbar = get_line_func("Crossbar");
    parts_->text = SG::FindTypedNodeUnderNode<SG::TextNode>(*this, "Text");
}

void LinearFeedback::ComputeFrame_(const Point3f &p0, const Vector3f &dir,
                                   float length, Frame_ &frame) {
    // Input values.
    frame.p0        = p0;
    frame.direction = dir;
    frame.length    = length;

    frame.p1 = p0 + length * dir;

    // Compute a reasonable "up" direction. XXXX
    frame.up_direction = Vector3f::AxisY();

    // Compute a minimum height in the up direction so the feedback will not
    // intersect anything in the scene, based on the scene bounds.
    const Bounds bounds = GetSceneBounds();
    Point3f max_bounds_pt = bounds.GetMaxPoint();
    for (int dim = 0; dim < 3; ++dim) {
        if (frame.up_direction[dim] < 0)
            max_bounds_pt[dim] = bounds.GetMinPoint()[dim];
    }
    const Plane plane(max_bounds_pt, frame.up_direction);
    const float min_height =
        kHeightMult_ * std::fabs(plane.GetDistanceToPoint(frame.p1));

    frame.crossbar_height = min_height + kExtraHeight_;
    frame.upright_length  = frame.crossbar_height + kExtensionLength_;
    frame.text_height     = frame.upright_length + kTextHeightOffset_;
}
