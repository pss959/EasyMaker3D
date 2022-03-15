#include "Panes/IconPane.h"

#include "Parser/Registry.h"
#include "SG/UniformBlock.h"
#include "Util/Assert.h"

void IconPane::AddFields() {
    AddField(icon_name_);
    Pane::AddFields();
}

bool IconPane::IsValid(std::string &details) {
    if (! Pane::IsValid(details))
        return false;

    if (icon_name_.GetValue().empty()) {
        details = "No icon name specified";
        return false;
    }

    return true;
}

void IconPane::CreationDone() {
    Pane::CreationDone();

    if (! IsTemplate()) {
        ASSERT(! GetUniformBlocks().empty());
        auto &block = GetUniformBlocks()[0];
        block->SetSubImageName(icon_name_);
    }
}

void IconPane::SetIconName(const std::string &name) {
    ASSERT(! name.empty());
    ASSERT(! GetUniformBlocks().empty());
    auto &block = GetUniformBlocks()[0];
    icon_name_ = name;
    block->SetSubImageName(name);
}

std::string IconPane::ToString() const {
    // Add the icon name.
    return Pane::ToString() + " '" + icon_name_.GetValue() + "'";
}
