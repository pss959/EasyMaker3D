#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Math/Types.h"

namespace SG {

DECL_SHARED_PTR(Node);

/// The RenderData struct encapsulates the data shared by RenderPass classes
/// during multipass rendering.
///
/// \ingroup SG
struct RenderData {
    /// Per-light data for rendering.
    struct LightData {
        Point3f              position;       ///< From light (world coords).
        Color                color;          ///< From light.
        bool                 casts_shadows;  ///< From light.
        Matrix4f             light_matrix;   ///< Computed by ShadowPass.
        ion::gfx::TexturePtr shadow_map;     ///< Generated by ShadowPass.
    };

    Range2i                viewport;         ///< From View.
    Matrix4f               proj_matrix;      ///< Computed from View.
    Matrix4f               view_matrix;      ///< Computed from View.
    Point3f                view_pos;         ///< Set from View.
    std::vector<LightData> per_light;        ///< LightData per light.

    SG::NodePtr            root_node;  ///< Root node of the graph to render.
};

}  // namespace SG
