#pragma once

#ifdef DEBUG  // Don't include this in release builds.

#include <string>

#include "Math/PolyMesh.h"
#include "Math/Types.h"
#include "Util/FilePath.h"
#include "Util/Flags.h"

namespace Debug {

/// (DEBUG builds only!) The Dump3dv class has several static methods for
/// dumping various items in 3dv format to files to aid with debugging.
class Dump3dv {
  public:
    /// Label flags.
    enum LabelFlag {
        kVertexLabels = (1 << 0),
        kEdgeLabels   = (1 << 1),
        kFaceLabels   = (1 << 2),
    };

    /// Dumps a TriMesh in 3dv format. If face_shrinkage is >= 0, each face in
    /// the mesh is shown as a 3dv face, shrunk by the given factor so that
    /// edges are also visible.
    static void DumpTriMesh(const TriMesh &mesh,
                            const std::string &description,
                            const FilePath &path,
                            float face_shrinkage = -1, bool add_labels = false);

    /// Dumps a PolyMesh in 3dv format. If face_shrinkage is >= 0, each face in
    /// the mesh is shown as a 3dv face, shrunk by the given factor so that
    /// edges are also visible.
    static void DumpPolyMesh(const PolyMesh &poly_mesh,
                             const std::string &description,
                             const FilePath &path,
                             float face_shrinkage = -1, int label_flags = 0);
};

}  // namespace Debug

#endif
