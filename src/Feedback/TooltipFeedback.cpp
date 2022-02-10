#include "Feedback/TooltipFeedback.h"

#include "Defaults.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/Delay.h"

// ----------------------------------------------------------------------------
// TooltipFeedback::Impl_ class definition.
// ----------------------------------------------------------------------------

class TooltipFeedback::Impl_ {
  public:
    Impl_(SG::Node &root_node) : root_node_(root_node) {}

    static void SetDelay(float seconds) { delay_ = seconds; }

    void InitParts();
    void SetUp(const std::string &text, const Point3f &position,
               const Rotationf &rotation);
    void SetColor(const Color &color) { /* XXXX */ }
    void Activate();
    void Deactivate();

  private:
    SG::Node        &root_node_;
    SG::NodePtr      background_;
    SG::TextNodePtr  text_;

    /// Amount to delay showing any tooltip.
    static float delay_;

    /// Set to true while tooltip is being delayed.
    bool is_delayed_ = false;

    /// ID returned by Util::RunDelayed(); used to cancel.
    int delay_id_;

    /// Shows or hides the tooltip.
    void Show_(bool show);
};

// ----------------------------------------------------------------------------
// TooltipFeedback::Impl_ functions.
// ----------------------------------------------------------------------------

float TooltipFeedback::Impl_::delay_ = Defaults::kTooltipDelay;

void TooltipFeedback::Impl_::InitParts() {
    text_       = SG::FindTypedNodeUnderNode<SG::TextNode>(root_node_, "Text");
    background_ = SG::FindNodeUnderNode(root_node_, "Background");
}

void TooltipFeedback::Impl_::SetUp(const std::string &text,
                                   const Point3f &position,
                                   const Rotationf &rotation) {
    text_->SetText(text);
    Vector3f size = text_->GetTextBounds().GetSize();
    size[0] *= 1.1f;
    size[1] *= 1.25f;
    background_->SetScale(size);

    root_node_.SetRotation(rotation);
    root_node_.SetTranslation(Vector3f(position));
}

void TooltipFeedback::Impl_::Activate() {
    if (! is_delayed_) {
        if (delay_ > 0) {
            delay_id_ = Util::RunDelayed(delay_, [this](){ Show_(true); });
            is_delayed_ = true;
        }
        else {
            Show_(true);
        }
    }
}

void TooltipFeedback::Impl_::Deactivate() {
    if (is_delayed_) {
        is_delayed_ = false;
        Util::CancelDelayed(delay_id_);
    }
    Show_(false);
}

void TooltipFeedback::Impl_::Show_(bool show) {
    root_node_.SetEnabled(show);
}

// ----------------------------------------------------------------------------
// TooltipFeedback functions.
// ----------------------------------------------------------------------------

TooltipFeedback::TooltipFeedback() : impl_(new Impl_(*this)) {
}

void TooltipFeedback::CreationDone() {
    Feedback::CreationDone();
    if (! IsTemplate())
        impl_->InitParts();
}

void TooltipFeedback::SetDelay(float seconds) {
    Impl_::SetDelay(seconds);
}

void TooltipFeedback::SetUp(const std::string &text, const Point3f &position,
                            const Rotationf &rotation) {
    impl_->SetUp(text, position, rotation);
}

void TooltipFeedback::SetColor(const Color &color) {
    impl_->SetColor(color);
}

void TooltipFeedback::Activate() {
    impl_->Activate();
}

void TooltipFeedback::Deactivate() {
    impl_->Deactivate();
}
