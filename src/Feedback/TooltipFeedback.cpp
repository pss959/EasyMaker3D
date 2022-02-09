#include "Feedback/TooltipFeedback.h"

#include "SG/Node.h"
#include "SG/TextNode.h"

// ----------------------------------------------------------------------------
// TooltipFeedback::Impl_ class definition.
// ----------------------------------------------------------------------------

class TooltipFeedback::Impl_ {
  public:
    Impl_(SG::Node &root_node) : root_node_(root_node) {}

    void InitParts();
    void SetColor(const Color &color) { /* XXXX */ }

  private:
    SG::Node        &root_node_;
    SG::TextNodePtr  text_;

    /// Set to true while tooltip is being delayed.
    bool is_delayed_ = false;

    /// ID returned by Util::RunDelayed(); used to cancel.
    int delay_id_;
};

// ----------------------------------------------------------------------------
// TooltipFeedback::Impl_ functions.
// ----------------------------------------------------------------------------

void TooltipFeedback::Impl_::InitParts() {
    // XXXX
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

void TooltipFeedback::SetColor(const Color &color) {
    impl_->SetColor(color);
}

void TooltipFeedback::Activate() {
    // XXXX
}

void TooltipFeedback::Deactivate() {
    // XXXX
}
