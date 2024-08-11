//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Color.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

Color Color::FromHSV(float h, float s, float v) {
    const float hs = (h == 1.f ? 0.f : 6.f * h);
    const int   hue_sextant = static_cast<int>(floorf(hs));
    const float hue_frac = hs - hue_sextant;

    const float t1 = v * (1.f - s);
    const float t2 = v * (1.f - (s * hue_frac));
    const float t3 = v * (1.f - (s * (1.f - hue_frac)));

    switch (hue_sextant) {
      case 0:
        return Color(v, t3, t1);
      case 1:
        return Color(t2, v, t1);
      case 2:
        return Color(t1, v, t3);
      case 3:
        return Color(t1, t2, v);
      case 4:
        return Color(t3, t1, v);
      default:
        return Color(v, t1, t2);
    }

}

Vector3f Color::ToHSV() const {
    const float r = (*this)[0];
    const float g = (*this)[1];
    const float b = (*this)[2];

    const float max = std::max(r, std::max(g, b));
    const float min = std::min(r, std::min(g, b));
    const float diff = max - min;

    const float val = max;
    const float sat = (max > 0 ? diff / max : 0);

    float hue = 0;
    if (sat > 0) {
	if (r == max)
	    hue = (g - b) / diff;
	else if (g == max)
	    hue = 2 + (b - r) / diff;
	else
	    hue = 4 + (r - g) / diff;
        if (hue < 0)
            hue += 6;
	hue /= 6;
    }

    return Vector3f(hue, sat, val);
}

Str Color::ToHexString() const {
    auto to_hex = [](float f){
        std::ostringstream out;
        out << std::hex << std::setfill('0') << std::setw(2)
            << std::lround(255 * f);
        return out.str();
    };
    const auto &v = *this;
    return "#" + to_hex(v[0]) + to_hex(v[1]) + to_hex(v[2]) + to_hex(v[3]);
}

bool Color::FromHexString(const Str &str) {
    if (str[0] != '#' || (str.size() != 7U && str.size() != 9U))
        return false;

    for (size_t i = 1; i < str.size(); ++i)
        if (! std::isxdigit(str[i]))
            return false;

    uint32_t n;
    std::istringstream(&str[1]) >> std::hex >> n;

    if (str.size() == 9) {  // #RRGGBBAA format.
        Set(static_cast<float>((n >> 24) & 0xff) / 255.f,
            static_cast<float>((n >> 16) & 0xff) / 255.f,
            static_cast<float>((n >>  8) & 0xff) / 255.f,
            static_cast<float>( n        & 0xff) / 255.f);
    }
    else if (str.size() == 7) {  // #RRGGBB format.
        Set(static_cast<float>((n >> 16) & 0xff) / 255.f,
            static_cast<float>((n >>  8) & 0xff) / 255.f,
            static_cast<float>( n        & 0xff) / 255.f,
            1.f);
    }
    return true;
}
