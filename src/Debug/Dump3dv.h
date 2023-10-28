#pragma once

#if ENABLE_DEBUG_FEATURES

#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "Math/Point3fMap.h"
#include "Math/PolyMesh.h"
#include "Math/Types.h"
#include "Util/FilePath.h"
#include "Util/Flags.h"

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

    using LabelFlags = Util::Flags<LabelFlag>;

    /// \name Functions used to highlight certain TriMesh features.
    ///@{
    using TFaceHighlightFunc   = std::function<bool(GIndex)>;
    using TEdgeHighlightFunc   = std::function<bool(GIndex, GIndex)>;
    using TVertexHighlightFunc = std::function<bool(GIndex)>;
    ///@}

    /// \name Functions used to highlight certain PolyMesh features.
    ///@{
    using PFaceHighlightFunc   = std::function<bool(const PolyMesh::Face   &)>;
    using PEdgeHighlightFunc   = std::function<bool(const PolyMesh::Edge   &)>;
    using PVertexHighlightFunc = std::function<bool(const PolyMesh::Vertex &)>;
    ///@}

    /// The constructor is passed the file to dump to and a string to put in
    /// the header comment.
    Dump3dv(const FilePath &path, const Str &header);

    /// The destructor finishes the dumped file.
    ~Dump3dv();

    /// Sets an extra prefix to add to all ID's in subsequent items.
    void SetExtraPrefix(const Str &extra_prefix) {
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

    /// Sets an offset vector to add to any label that occupies the same
    /// position as another (within a reasonable tolerance). The default is
    /// (0,0,.1).
    void SetCoincidentLabelOffset(const Vector3f &offset) {
        coincident_label_offset_ = offset;
    }

    /// Sets an extra offset to add to all labels. The default is 0.
    void SetExtraLabelOffset(const Vector3f &offset) {
        extra_label_offset_ = offset;
    }

    /// Adds a TriMesh to dump. The highlight functions, if supplied, are used
    /// to call out certain features with highlight colors.
    void AddTriMesh(const TriMesh &mesh,
                    const TFaceHighlightFunc &face_highlight_func = nullptr,
                    const TEdgeHighlightFunc &edge_highlight_func = nullptr,
                    const TVertexHighlightFunc &vert_highlight_func = nullptr);

    /// Adds a PolyMesh to dump. The highlight functions, if supplied, are used
    /// to call out certain features with highlight colors.
    void AddPolyMesh(const PolyMesh &mesh,
                     const PFaceHighlightFunc &face_highlight_func = nullptr,
                     const PEdgeHighlightFunc &edge_highlight_func = nullptr,
                     const PVertexHighlightFunc &vert_highlight_func = nullptr);

    /// Adds a vertex to dump. Also adds a label if the vertex labeling flag is
    /// set.
    void AddVertex(const Str &id, const Point3f &point);

    /// Adds an edge between two vertices to dump. Also adds a label if the
    /// edge labeling flag is set.
    void AddEdge(const Str &id, const Str &v0_id, const Str &v1_id);

    /// Adds a face joining the given vertices to dump. Adds a label if the
    /// face labeling flag is set.
    void AddFace(const Str &id, const StrVec &vids);

  private:
    std::ofstream out_;
    LabelFlags label_flags_;
    float      label_font_size_;
    Vector3f   coincident_label_offset_{0, 0, .1f};
    Vector3f   extra_label_offset_{0, 0, 0};

    /// Used to detect coincident labels.
    Point3fMap label_point_map_{.001f};

    /// Current extra prefix to add to IDs.
    Str        extra_prefix_;

    /// Adds a 3dv vertex statement.
    void AddVertex_(const Str &id, const Point3f &p);

    /// Adds a 3dv text statement to show a label.
    void AddLabel_(const Point3f &pos, const Str &text);

    /// Adds a 3dv rotated text statement to show a label facing in the given
    /// direction.
    void AddRotatedLabel_(const Point3f &pos, const Vector3f &dir,
                          const Str &text);

    /// Outputs an alternating color for a face depending on an index.
    void AltFaceColor_(size_t i);

    /// Creates an ID string from a string, adding the extra prefix.
    Str ID_(const Str &id);

    /// Creates an ID string from a prefix and index, adding the extra prefix.
    Str IID_(const Str &prefix, int index);
};

}  // namespace Debug

#endif
