#include "Feedback/AngularFeedback.h"

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "SG/Line.h"
#include "SG/Node.h"
#include "SG/PolyLine.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// AngularFeedback::Impl_ class definition.
// ----------------------------------------------------------------------------

class AngularFeedback::Impl_ {
  public:
    Impl_(SG::Node &root_node) : root_node_(root_node) {}

    void InitParts();
    void SetColor(const Color &color) { color_ = color; }
    void SubtendAngle(const Point3f &center, float up_offset,
                      float text_up_offset, const Rotationf &text_rotation,
                      const Vector3f &axis,
                      const Anglef &start_angle, const Anglef &end_angle);

  private:
    struct Frame_;

    /// This struct stores all of the parts the AngularFeedback needs to operate.
    struct Parts_ {
        SG::LinePtr     start_line;  ///< Line showing the starting angle.
        SG::LinePtr     end_line;    ///< Line showing the ending angle.
        SG::PolyLinePtr arc;         ///< Circular arc subtending the angle.
        SG::TextNodePtr text;        ///< TextNode showing the feedback value.
    };

    // Various constants used to adjust the look of the feedback.
    static constexpr float kLineLength_           = 24;
    static constexpr float kArcRadius_            = 10.f;
    static constexpr float kArcDegreesPerSegment_ = 4;

    SG::Node &root_node_;
    Parts_    parts_;

    Color color_{ Color::White() };

    void FindParts_();
    void UpdateLines_(const Anglef &start_angle, const Anglef &end_angle);
    void UpdateArc_(const Anglef &start_angle, const Anglef &arc_angle);
    void UpdateText_(const Anglef &arc_angle, float up_offset,
                     const Rotationf &rotation);
};

// ----------------------------------------------------------------------------
// AngularFeedback::Impl_ functions.
// ----------------------------------------------------------------------------

void AngularFeedback::Impl_::InitParts() {
    auto get_line_func = [&](const std::string &name){
        SG::NodePtr n = SG::FindNodeUnderNode(root_node_, name);
        return SG::FindTypedShapeInNode<SG::Line>(*n, "Line");
    };
    auto get_polyline_func = [&](const std::string &name){
        SG::NodePtr n = SG::FindNodeUnderNode(root_node_, name);
        return SG::FindTypedShapeInNode<SG::PolyLine>(*n, "Line");
    };

    parts_.start_line = get_line_func("StartLine");
    parts_.end_line   = get_line_func("EndLine");
    parts_.arc        = get_polyline_func("Arc");
    parts_.text = SG::FindTypedNodeUnderNode<SG::TextNode>(root_node_, "Text");
}

void AngularFeedback::Impl_::SubtendAngle(const Point3f &center,
                                          float up_offset, float text_up_offset,
                                          const Rotationf &text_rotation,
                                          const Vector3f &axis,
                                          const Anglef &start_angle,
                                          const Anglef &end_angle) {
    // Rotate the feedback so that it is perpendicular to the axis.
    root_node_.SetRotation(Rotationf::RotateInto(axis, Vector3f::AxisZ()));

    // Position based on the center and offset.
    root_node_.SetTranslation(center + Vector3f(0, up_offset, 0));

    // Modify the angles if necessary.
    const Anglef arc_angle = end_angle - start_angle;
    float rounded_degrees = RoundToPrecision(arc_angle.Degrees(), 1);
    if (rounded_degrees >= 360)
        rounded_degrees -= 360;

    const Anglef adjusted_arc_angle = Anglef::FromDegrees(rounded_degrees);
    const Anglef adjusted_end_angle = start_angle + adjusted_arc_angle;

    // Update the parts.
    UpdateLines_(start_angle, adjusted_end_angle);
    UpdateArc_(start_angle, adjusted_arc_angle);
    UpdateText_(adjusted_arc_angle, text_up_offset, text_rotation);

    root_node_.SetBaseColor(color_);
}

void AngularFeedback::Impl_::UpdateLines_(const Anglef &start_angle,
                                          const Anglef &end_angle) {
    auto get_end_pt = [&](const Anglef &angle){
        const Rotationf rot =
            Rotationf::FromAxisAndAngle(Vector3f::AxisZ(), angle);
        return rot * Point3f(kLineLength_, 0, 0);
    };

    parts_.start_line->SetEndpoints(Point3f::Zero(), get_end_pt(start_angle));
    parts_.end_line->SetEndpoints(Point3f::Zero(), get_end_pt(end_angle));
}

void AngularFeedback::Impl_::UpdateArc_(const Anglef &start_angle,
                                        const Anglef &arc_angle) {
    const int seg_count = static_cast<int>(
        std::abs(arc_angle.Degrees()) / kArcDegreesPerSegment_);
    if (! seg_count)
        return;
    const Anglef seg_angle = arc_angle / seg_count;

    const int pt_count = 1 + seg_count;
    std::vector<Point3f> points(pt_count);
    for (int i = 0; i < pt_count; ++i) {
        const Anglef angle = i * seg_angle;
        points[i].Set(kArcRadius_ * ion::math::Cosine(angle),
                      kArcRadius_ * ion::math::Sine(angle), 0);
    }
    parts_.arc->SetPoints(points);
}

void AngularFeedback::Impl_::UpdateText_(const Anglef &angle, float up_offset,
                                         const Rotationf &rotation) {
    auto &text = *parts_.text;
    text.SetTranslation(Vector3f(4, up_offset, 0));
    text.SetRotation(rotation);
    text.SetTextWithColor(Util::ToString(angle.Degrees()), color_);
}

// ----------------------------------------------------------------------------
// AngularFeedback functions.
// ----------------------------------------------------------------------------

AngularFeedback::AngularFeedback() : impl_(new Impl_(*this)) {
}

void AngularFeedback::CreationDone() {
    Feedback::CreationDone();
    if (! IsTemplate())
        impl_->InitParts();
}

void AngularFeedback::SetColor(const Color &color) {
    impl_->SetColor(color);
}

void AngularFeedback::SubtendAngle(const Point3f &center, float up_offset,
                                   float text_up_offset, const Vector3f &axis,
                                   const Anglef &start_angle,
                                   const Anglef &end_angle) {
    impl_->SubtendAngle(center, up_offset, text_up_offset,
                        -GetRotation() * GetTextRotation(),
                        axis, start_angle, end_angle);
}
