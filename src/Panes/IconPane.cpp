//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/IconPane.h"

#include "Parser/Registry.h"
#include "SG/UniformBlock.h"
#include "Util/Assert.h"

void IconPane::AddFields() {
    AddField(icon_name_.Init("icon_name"));

    LeafPane::AddFields();
}

bool IconPane::IsValid(Str &details) {
    if (! LeafPane::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (icon_name_.GetValue().empty()) {
        details = "No icon name specified";
        return false;
    }
    return true;
}

void IconPane::CreationDone() {
    LeafPane::CreationDone();

    if (! IsTemplate()) {
        ASSERT(! GetUniformBlocks().empty());
        auto &block = GetUniformBlocks()[0];
        block->SetSubImageName(icon_name_);
    }
}

void IconPane::SetIconName(const Str &name) {
    ASSERT(! name.empty());
    ASSERT(! GetUniformBlocks().empty());
    auto &block = GetUniformBlocks()[0];
    icon_name_ = name;
    block->SetSubImageName(name);
}

// LCOV_EXCL_START [debug only]
Str IconPane::ToString(bool is_brief) const {
    // Add the icon name.
    return LeafPane::ToString(is_brief) + " '" + icon_name_.GetValue() + "'";
}
// LCOV_EXCL_STOP
