//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/ColorMap.h"

#include "Util/Assert.h"

namespace SG {

const ColorMap * ColorMap::s_instance = nullptr;

void ColorMap::Reset() {
    s_instance = nullptr;
}

void ColorMap::AddFields() {
    AddField(named_colors_.Init("named_colors"));

    Object::AddFields();
}

void ColorMap::CreationDone() {
    // Set up the map.
    for (const auto &nc: GetNamedColors()) {
        ASSERT(! map_.contains(nc->GetName()));
        map_[nc->GetName()] = nc->GetColor();
    }

    // Store this as the static instance.
    s_instance = this;
}

Color ColorMap::GetColor(const Str &name) const {
    ASSERTM(map_.contains(name), "Missing color: " + name);
    return map_.at(name);
}

Color ColorMap::GetColorForDimension(int dim) const {
    ASSERT(dim >= 0 && dim <= 2);
    Str name = "DimensionColor";
    name += 'X' + dim;
    return GetColor(name);
}

Color ColorMap::SGetColor(const Str &name) {
    return s_instance ? s_instance->GetColor(name) : Color::White();
}

Color ColorMap::SGetColorForDimension(int dim) {
    ASSERT(s_instance);
    return s_instance->GetColorForDimension(dim);
}

}  // namespace SG
