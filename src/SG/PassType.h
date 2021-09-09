#pragma once

namespace SG {

//! The PassType enum represents a type of render pass being applied. These can
//! be used to restrict actions to specific passes.
enum class PassType {
    kAnyPass,
    kShadowPass,
    kLightingPass,
};

//! Returns true if PassType t0 is contained in PassType t1.
inline bool PassIn(PassType t0, PassType t1) {
    return t1 == PassType::kAnyPass || t0 == t1;
}

//! Returns true if two PassType values match.
inline bool PassesMatch(PassType t0, PassType t1) {
    return t0 == PassType::kAnyPass || t1 == PassType::kAnyPass || t0 == t1;
}

}  // namespace SG
