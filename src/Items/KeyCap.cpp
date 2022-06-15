#include "Items/KeyCap.h"

#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Util/String.h"

void KeyCap::AddFields() {
    AddField(width_);
    AddField(text_);
    AddField(shifted_text_);
    SG::Node::AddFields();
}

bool KeyCap::IsValid(std::string &details) {
    if (! SG::Node::IsValid(details))
        return false;

    if (width_ <= 0) {
        details = "Non-positive width";
        return false;
    }

    return true;
}

void KeyCap::CreationDone() {
    SG::Node::CreationDone();

    if (! IsTemplate()) {
        // Scale the geometry based on the width.
        if (GetWidth() != 1)
            SG::FindNodeUnderNode(*this, "Geometry")->SetScale(
                Vector3f(GetWidth(), 1, 1));

        UpdateLabelText(false);
    }
}

std::string KeyCap::GetText(bool is_shifted) {
    std::string s;
    if (is_shifted) {
        // If specific shifted text is supplied, use it. Otherwise, just
        // convert the regular text to upper case.
        s = shifted_text_;
        if (s.empty())
            s = Util::ToUpperCase(GetText(false));
    }
    else {
        // If specific text is supplied, use it. Otherwise, just use the name
        // converted to lower case.
        s = text_;
        if (s.empty())
            s = Util::ToLowerCase(GetName());
    }
    return s;
}

void KeyCap::UpdateLabelText(bool is_shifted) {
    auto node = SG::FindTypedNodeUnderNode<SG::TextNode>(*this, "LabelText");
    node->SetText(GetText(is_shifted));
}
