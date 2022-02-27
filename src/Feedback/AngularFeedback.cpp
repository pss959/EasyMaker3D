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
                      float text_up_offset, const Rotationf &orientation,
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
    static constexpr float kTextScale_            = .6f;

    SG::Node &root_node_;
    Parts_    parts_;

    Color color_{ Color::White() };

    void FindParts_();
    void UpdateLines_(const Anglef &start_angle, const Anglef &end_angle);
    void UpdateArc_(const Anglef &start_angle, const Anglef &end_angle);
    void UpdateText_(const Anglef &angle, float up_offset);
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
                                   float text_up_offset,
                                   const Rotationf &orientation,
                                   const Anglef &start_angle,
                                   const Anglef &end_angle) {
    // XXXX
}
