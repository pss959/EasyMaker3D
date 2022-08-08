#include "Feedback/AngularFeedback.h"

#include <ion/math/vectorutils.h>

#include "Base/Tuning.h"
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
    void SubtendArc(const Point3f &center, float up_offset,
                    float text_up_offset, float text_scale,
                    const Rotationf &text_rotation,
                    const Vector3f &axis, const CircleArc &arc);

  private:
    struct Frame_;

    /// This struct stores all of the parts the AngularFeedback needs to operate.
    struct Parts_ {
        SG::LinePtr     start_line;  ///< Line showing the starting angle.
        SG::LinePtr     end_line;    ///< Line showing the ending angle.
        SG::PolyLinePtr arc;         ///< Circular arc subtending the angle.
        SG::TextNodePtr text;        ///< TextNode showing the feedback value.
    };

    SG::Node &root_node_;
    Parts_    parts_;

    Color color_{ Color::White() };

    void FindParts_();
    void UpdateLines_(const CircleArc &arc);
    void UpdateArc_(const CircleArc &arc);
    void UpdateText_(const Anglef &arc_angle, float up_offset, float scale,
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

void AngularFeedback::Impl_::SubtendArc(const Point3f &center,
                                        float up_offset, float text_up_offset,
                                        float text_scale,
                                        const Rotationf &text_rotation,
                                        const Vector3f &axis,
                                        const CircleArc &arc) {
    // Rotate the feedback so that it is perpendicular to the axis.
    root_node_.SetRotation(Rotationf::RotateInto(Vector3f::AxisZ(), axis));

    // Position based on the center and offset.
    root_node_.SetTranslation(center + Vector3f(0, up_offset, 0));

    // Modify the angles if necessary.
    float rounded_degrees = RoundToPrecision(arc.arc_angle.Degrees(), 1);
    if (rounded_degrees >= 360)
        rounded_degrees -= 360;
    const CircleArc adjusted_arc(arc.start_angle,
                                 Anglef::FromDegrees(rounded_degrees));

    // Update the parts.
    UpdateLines_(adjusted_arc);
    UpdateArc_(adjusted_arc);
    UpdateText_(adjusted_arc.arc_angle, text_up_offset,
                text_scale, text_rotation);

    root_node_.SetBaseColor(color_);
}

void AngularFeedback::Impl_::UpdateLines_(const CircleArc &arc) {
    auto get_end_pt = [&](const Anglef &angle){
        const Rotationf rot =
            Rotationf::FromAxisAndAngle(Vector3f::AxisZ(), angle);
        return rot * Point3f(TK::kAngularFeedbackLineLength, 0, 0);
    };

    parts_.start_line->SetEndpoints(Point3f::Zero(),
                                    get_end_pt(arc.start_angle));
    parts_.end_line->SetEndpoints(Point3f::Zero(),
                                  get_end_pt(arc.start_angle + arc.arc_angle));
}

void AngularFeedback::Impl_::UpdateArc_(const CircleArc &arc) {
    parts_.arc->SetArcPoints(arc, TK::kAngularFeedbackArcRadius,
                             TK::kAngularFeedbackDegreesPerSegment);
}

void AngularFeedback::Impl_::UpdateText_(const Anglef &angle, float up_offset,
                                         float scale,
                                         const Rotationf &rotation) {
    auto &text = *parts_.text;
    text.SetUniformScale(scale);
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

void AngularFeedback::SubtendArc(const Point3f &center, float up_offset,
                                 float text_up_offset, const Vector3f &axis,
                                 const CircleArc &arc) {
    const float scale = GetTextScale();
    impl_->SubtendArc(center, up_offset, scale * text_up_offset, scale,
                      -GetRotation() * GetTextRotation(), axis, arc);
}
