#include "SG/ColorMap.h"

#include "Util/Assert.h"
#include "Util/General.h"

namespace SG {

const ColorMap * ColorMap::s_instance = nullptr;

void ColorMap::AddFields() {
    AddField(named_colors_.Init("named_colors"));

    Object::AddFields();
}

void ColorMap::CreationDone() {
    // Set up the map.
    for (const auto &nc: GetNamedColors()) {
        ASSERT(! Util::MapContains(map_, nc->GetName()));
        map_[nc->GetName()] = nc->GetColor();
    }

    // Store this as the static instance.
    s_instance = this;
}

Color ColorMap::GetColor(const std::string &name) const {
    ASSERTM(Util::MapContains(map_, name), "Missing color: " + name);
    return map_.at(name);
}

Color ColorMap::GetColorForDimension(int dim) const {
    ASSERT(dim >= 0 && dim <= 2);
    std::string name = "DimensionColor";
    name += 'X' + dim;
    return GetColor(name);
}

Color ColorMap::SGetColor(const std::string &name) {
    ASSERT(s_instance);
    return s_instance->GetColor(name);
}

Color ColorMap::SGetColorForDimension(int dim) {
    ASSERT(s_instance);
    return s_instance->GetColorForDimension(dim);
}

}  // namespace SG
