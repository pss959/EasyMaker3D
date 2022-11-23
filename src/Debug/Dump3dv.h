#pragma once

#if ENABLE_DEBUG_FEATURES

#include <fstream>
#include <string>
#include <vector>

#include "Math/Types.h"
#include "Util/FilePath.h"
#include "Util/Flags.h"

struct PolyMesh;
class  Skeleton3D;

namespace Debug {

/// (Non-release builds only!) The Dump3dv class has several functions for
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

    /// The constructor is passed the file to dump to and a string to put in
    /// the header comment.
    Dump3dv(const FilePath &path, const std::string &header);

    /// The destructor finishes the dumped file.
    ~Dump3dv();

    /// Sets an extra prefix to add to all ID's in subsequent items.
    void SetExtraPrefix(const std::string &extra_prefix) {
        extra_prefix_ = extra_prefix;
    }

    /// Sets the label flags. The default is all flags true.
    void SetLabelFlags(const LabelFlags &label_flags) {
        label_flags_ = label_flags;
    }

    /// Sets the label font size. The default is 20.
    void SetLabelFontSize(float label_font_size) {
        label_font_size_ = label_font_size;
    }

    /// Sets an extra offset to add to all labels. The default is 0.
    void SetLabelOffset(const Vector3f &offset) {
        label_offset_ = offset;
    }

    /// Dumps a TriMesh.
    void AddTriMesh(const TriMesh &mesh);

    /// Dumps a PolyMesh.
    void AddPolyMesh(const PolyMesh &mesh);

    /// Dumps a Skeleton3D.
    void AddSkeleton3D(const Skeleton3D &skel);

  private:
    std::ofstream out_;
    LabelFlags    label_flags_;
    float         label_font_size_;
    Vector3f      label_offset_;

    /// Current extra prefix to add to IDs.
    std::string   extra_prefix_;

    /// Adds a 3dv vertex statement.
    void AddVertex_(const std::string &id, const Point3f &p);

    /// Adds a 3dv label statement.
    void AddLabel_(const Point3f &pos, const std::string &text);

    /// Outputs an alternating color for a face depending on an index.
    void AltFaceColor_(size_t i);

    /// Creates an ID string from a string, adding the extra prefix.
    std::string ID_(const std::string &id);

    /// Creates an ID string from a prefix and index, adding the extra prefix.
    std::string IID_(const std::string &prefix, int index);
};

}  // namespace Debug

#endif
