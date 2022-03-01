#include "Feedback/LinearFeedback.h"

#include <algorithm>
#include <cmath>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "SG/Line.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// LinearFeedback::Impl_ class definition.
// ----------------------------------------------------------------------------

class LinearFeedback::Impl_ {
  public:
    Impl_(SG::Node &root_node) : root_node_(root_node) {}

    void InitParts();
    void SetColor(const Color &color) { color_ = color; }
    void SpanLength(const Point3f &pt, const Vector3f &dir, float length,
                    const Bounds &scene_bounds, const Rotationf &text_rotation);

  private:
    struct Frame_;

    /// This struct stores all of the parts the LinearFeedback needs to operate.
    struct Parts_ {
        SG::LinePtr     upright0;   ///< Upright connected to p0.
        SG::LinePtr     upright1;   ///< Upright connected to p1.
        SG::LinePtr     crossbar;   ///< Crossbar connecting the uprights.
        SG::TextNodePtr text;       ///< TextNode showing the feedback value.
    };

    // Various constants used to adjust the look of the feedback.
    static constexpr float kExtraHeight_      = .5f;
    static constexpr float kExtensionLength_  = 2.f;
    static constexpr float kHeightMult_       = 1.1f;
    static constexpr float kTextHeightOffset_ = 1.f;
    static constexpr float kMinTextY_         = 1.f;

    SG::Node &root_node_;
    Parts_    parts_;

    Color color_{ Color::White() };

    void FindParts_();
    void ComputeFrame_(const Point3f &p0, const Vector3f &dir, float length,
                       const Bounds &scene_bounds, Frame_ &frame);
};

// ----------------------------------------------------------------------------
// LinearFeedback::Impl_::Frame_ struct.
// ----------------------------------------------------------------------------

/// This struct stores everything needed to draw linear feedback. A transient
/// instance is created when necessary to update the parts of the feedback.
struct LinearFeedback::Impl_::Frame_ {
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
// LinearFeedback::Impl_ functions.
// ----------------------------------------------------------------------------

void LinearFeedback::Impl_::InitParts() {
    auto get_line_func = [&](const std::string &name){
        SG::NodePtr n = SG::FindNodeUnderNode(root_node_, name);
        return SG::FindTypedShapeInNode<SG::Line>(*n, "Line");
    };

    parts_.upright0 = get_line_func("Upright0");
    parts_.upright1 = get_line_func("Upright1");
    parts_.crossbar = get_line_func("Crossbar");
    parts_.text = SG::FindTypedNodeUnderNode<SG::TextNode>(root_node_, "Text");
}

void LinearFeedback::Impl_::SpanLength(const Point3f &pt, const Vector3f &dir,
                                       float length,
                                       const Bounds &scene_bounds,
                                       const Rotationf &text_rotation) {
    Frame_ frame;
    ComputeFrame_(pt, dir, length, scene_bounds, frame);
    const Vector3f upright_vec = frame.upright_length * frame.up_direction;

    // Update the upright lines.
    parts_.upright0->SetEndpoints(frame.p0, frame.p0 + upright_vec);
    parts_.upright1->SetEndpoints(frame.p1, frame.p1 + upright_vec);

    // Update the crossbar line.
    const Vector3f crossbar_up = frame.crossbar_height * frame.up_direction;
    parts_.crossbar->SetEndpoints(frame.p0 + crossbar_up,
                                  frame.p1 + crossbar_up);

    // Update the text. Do NOT let it text go below the minimum.
    Point3f text_pos = frame.p1 + frame.text_height * frame.up_direction;
    text_pos[1] = std::max(text_pos[1], kMinTextY_);
    parts_.text->SetTranslation(text_pos);
    parts_.text->SetRotation(text_rotation);
    parts_.text->SetTextWithColor(
        Util::ToString(std::roundf(100 * frame.length) / 100), color_);

    // Set the color for all other parts.
    root_node_.SetBaseColor(color_);
}

void LinearFeedback::Impl_::ComputeFrame_(const Point3f &p0,
                                          const Vector3f &dir, float length,
                                          const Bounds &scene_bounds,
                                          Frame_ &frame) {
    // Input values.
    frame.p0        = p0;
    frame.direction = dir;
    frame.length    = length;

    frame.p1 = p0 + length * dir;

    // Compute a reasonable "up" direction. If the direction of motion is close
    // enough to the Y axis, use the X axis. Otherwise, use Y.
    const int up_dim = std::fabs(dir[1]) > .9f ? 0 : 1;
    frame.up_direction = GetAxis(up_dim);

    // Compute a minimum height in the up direction so the feedback will not
    // intersect anything in the scene, based on the scene bounds.
    const float min_height =
        kHeightMult_ * (scene_bounds.GetMaxPoint()[up_dim] - p0[up_dim]);

    frame.crossbar_height = min_height + kExtraHeight_;
    frame.upright_length  = frame.crossbar_height + kExtensionLength_;
    frame.text_height     = frame.upright_length + kTextHeightOffset_;
}

// ----------------------------------------------------------------------------
// LinearFeedback functions.
// ----------------------------------------------------------------------------

LinearFeedback::LinearFeedback() : impl_(new Impl_(*this)) {
}

void LinearFeedback::CreationDone() {
    Feedback::CreationDone();
    if (! IsTemplate())
        impl_->InitParts();
}

void LinearFeedback::SetColor(const Color &color) {
    impl_->SetColor(color);
}

void LinearFeedback::SpanPoints(const Point3f &p0, const Point3f &p1) {
    const float length = ion::math::Distance(p0, p1);
    ASSERT(length > 0);
    SpanLength(p0, ion::math::Normalized(p1 - p0), length);
}

void LinearFeedback::SpanLength(const Point3f &pt, const Vector3f &dir,
                                float length) {
    impl_->SpanLength(pt, dir, length, GetSceneBounds(),
                      -GetRotation() * GetTextRotation());
}
