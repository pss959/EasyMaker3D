//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Feedback/AngularFeedback.h"

#include <ion/math/vectorutils.h>

#include "Math/Color.h"
#include "Math/Linear.h"
#include "SG/ColorMap.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "SG/Tube.h"
#include "Util/String.h"
#include "Util/Tuning.h"

// ----------------------------------------------------------------------------
// AngularFeedback::Impl_ class definition.
// ----------------------------------------------------------------------------

class AngularFeedback::Impl_ {
  public:
    Impl_(SG::Node &root_node) : root_node_(root_node) {}

    void InitParts();
    void SetColor(const Color &color) { color_ = color; }
    void SubtendArc(const Point3f &center, float up_offset,
                    float text_up_offset, const Matrix4f &owm,
                    const Vector3f &axis, const CircleArc &arc);

  private:
    struct Frame_;

    /// This struct stores all of the parts the AngularFeedback needs to operate.
    struct Parts_ {
        SG::TubePtr     start_line;  ///< Line showing the starting angle.
        SG::TubePtr     end_line;    ///< Line showing the ending angle.
        SG::TubePtr     arc;         ///< Circular arc subtending the angle.
        SG::TextNodePtr text;        ///< TextNode showing the feedback value.
    };

    SG::Node &root_node_;
    Parts_    parts_;

    Color color_{SG::ColorMap::SGetColor("FeedbackNeutralColor")};

    void FindParts_();
    void UpdateLines_(const CircleArc &arc);
    void UpdateArc_(const CircleArc &arc);
    void UpdateText_(const Anglef &angle, float up_offset, const Matrix4f &owm);
};

// ----------------------------------------------------------------------------
// AngularFeedback::Impl_ functions.
// ----------------------------------------------------------------------------

void AngularFeedback::Impl_::InitParts() {
    auto get_tube_func = [&](const Str &name){
        SG::NodePtr n = SG::FindNodeUnderNode(root_node_, name);
        return SG::FindTypedShapeInNode<SG::Tube>(*n, "Line");
    };

    parts_.start_line = get_tube_func("StartLine");
    parts_.end_line   = get_tube_func("EndLine");
    parts_.arc        = get_tube_func("Arc");
    parts_.text = SG::FindTypedNodeUnderNode<SG::TextNode>(root_node_, "Text");
}

void AngularFeedback::Impl_::SubtendArc(const Point3f &center,
                                        float up_offset, float text_up_offset,
                                        const Matrix4f &owm,
                                        const Vector3f &axis,
                                        const CircleArc &arc) {
    // Rotate the feedback so that it is perpendicular to the axis.
    root_node_.SetRotation(Rotationf::RotateInto(Vector3f::AxisZ(), axis));

    // Position based on the center and offset.
    root_node_.TranslateTo(center + Vector3f(0, up_offset, 0));

    // Modify the angles if necessary.
    const float rounded_degrees =
        RoundToPrecision(arc.arc_angle.Degrees(), 1);
    const CircleArc adjusted_arc(arc.start_angle,
                                 Anglef::FromDegrees(rounded_degrees));

    // Round to the nearest angle for the text.
    const Anglef rounded_angle =
        Anglef::FromDegrees(RoundToPrecision(arc.arc_angle.Degrees(), 1));

    // Update the parts.
    UpdateLines_(arc);
    UpdateArc_(arc);
    UpdateText_(rounded_angle, text_up_offset, owm);

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
                                         const Matrix4f &owm) {
    auto &text = *parts_.text;

    // Set the scale and rotation for the text to face the camera.
    parts_.text->SetWorldScaleAndRotation(owm, TK::kAngularFeedbackTextScale,
                                          root_node_.GetRotation());
    text.SetTranslation(Vector3f(4, parts_.text->GetScale()[1] * up_offset, 0));
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
    impl_->SubtendArc(center, up_offset, text_up_offset,
                      GetObjectToWorldMatrix(), axis, arc);
}
