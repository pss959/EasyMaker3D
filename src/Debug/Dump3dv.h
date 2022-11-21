#pragma once

#if ENABLE_DEBUG_FEATURES

#include <string>

#include "Math/PolyMesh.h"
#include "Math/Types.h"
#include "Util/FilePath.h"
#include "Util/Flags.h"

namespace Debug {

/// (DEBUG builds only!) The Dump3dv class has several static methods for
/// dumping various items in 3dv format to files to aid with debugging.
///
/// \ingroup Debug
class Dump3dv {
  public:
    /// Label flags.
    enum LabelFlag {
        kVertexLabels = (1 << 0),
        kEdgeLabels   = (1 << 1),
        kFaceLabels   = (1 << 2),
    };

    typedef Util::Flags<LabelFlag> LabelFlags;

    /// Dumps a TriMesh in 3dv format. Labels are added according to the flags.
    static void DumpTriMesh(const TriMesh &mesh,
                            const std::string &description,
                            const FilePath &path,
                            const LabelFlags &label_flags);

    /// Dumps a PolyMesh in 3dv format. If face_shrinkage is >= 0, each face in
    /// the mesh is shown as a 3dv face, shrunk by the given factor so that
    /// edges are also visible.
    static void DumpPolyMesh(const PolyMesh &poly_mesh,
                             const std::string &description,
                             const FilePath &path,
                             const LabelFlags &label_flags);
};

}  // namespace Debug

#endif
