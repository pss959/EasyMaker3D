#include "SG/Init.h"

#include <ion/gfx/node.h>
#include <ion/gfx/shaderinputregistry.h>

#include "Assert.h"

using ion::gfx::ShaderInputRegistry;

namespace SG {

static bool s_initialized_ = false;

void Init() {
    if (s_initialized_)
        return;

    s_initialized_ = true;

    // Instead of using the built-in uModelviewMatrix, we use uModelMatrix and
    // uViewMatrix separately. This allows conversions to world coordinates
    // (with just the uModelMatrix). The uViewMatrix should never need to be
    // combined (set once per view). The uModelMatrix is combined the same way
    // uModelviewMatrix is, accumulating during graph traversal. Make sure to
    // do this only once, as it modifies the registry.
    auto &reg = ShaderInputRegistry::GetGlobalRegistry();

    auto *mv_spec = reg->Find<ion::gfx::Uniform>("uModelviewMatrix");
    ASSERT(mv_spec);

    reg->Add(ShaderInputRegistry::UniformSpec(
                 "uModelMatrix", ion::gfx::kMatrix4x4Uniform,
                 "Cumulative model matrix,", mv_spec->combine_function));
    reg->Add(ShaderInputRegistry::UniformSpec(
                 "uViewMatrix", ion::gfx::kMatrix4x4Uniform, "View matrix,"));
}

}  // namespace SG
