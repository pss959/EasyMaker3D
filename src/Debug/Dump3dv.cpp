#include "Debug/Dump3dv.h"

#include <random>

#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// \name Colors.
///@{
static const std::string kEdgeColor{"1 1 0 1"};
static const std::string kEdgeLabelColor{"1 1 0 1"};
static const std::string kFaceLabelColor{".7 .9 .8 1"};
static const std::string kHighlightEdgeColor{"1 .2 .8 1"};
static const std::string kHighlightEdgeLabelColor{"1 .8 .8 1"};
static const std::string kHighlightFaceLabelColor{"1 .5 .9 1"};
static const std::string kHighlightVertexLabelColor{"1 .5 .5 1"};
static const std::string kVertexLabelColor{"1 1 1 1"};
///@}

/// Outputs a Point3f with rounded precision.
inline std::ostream & operator<<(std::ostream &out, const Point3f &p) {
    for (int i = 0; i < 3; ++i)
        out << ' ' << RoundToPrecision(p[i], .01f);
    return out;
}

/// Outputs a Vector3f.
inline std::ostream & operator<<(std::ostream &out, const Vector3f &v) {
    out << v[0] << ' ' << v[1] << ' ' << v[2];
    return out;
}

/// Returns the center point of a TriMesh triangle.
static Point3f GetTriCenter_(const TriMesh &mesh, size_t tri_index) {
    return (mesh.points[mesh.indices[3 * tri_index + 0]] +
            mesh.points[mesh.indices[3 * tri_index + 1]] +
            mesh.points[mesh.indices[3 * tri_index + 2]]) / 3.f;
}

/// Returns the normal to a TriMesh triangle.
static Vector3f GetTriNormal_(const TriMesh &mesh, size_t tri_index) {
    return ComputeNormal(mesh.points[mesh.indices[3 * tri_index + 0]],
                         mesh.points[mesh.indices[3 * tri_index + 1]],
                         mesh.points[mesh.indices[3 * tri_index + 2]]);
}

/// Returns the center point of a PolyMesh::Face.
static Point3f GetFaceCenter_(const PolyMesh::Face &face) {
    Point3f center(0, 0, 0);
    for (auto &edge: face.outer_edges)
        center += edge->v0->point;
    center /= face.outer_edges.size();
    return center;
}

/// Returns the position of a label for a PolyMesh::Edge.
static Point3f GetEdgeLabelPos_(const PolyMesh::Edge &edge) {
    return edge.v0->point + .3f * (edge.v1->point - edge.v0->point);
}

/// Returns the approximate center point of a PolyMesh.
static Point3f GetPolyMeshCenter_(const PolyMesh &mesh) {
    Point3f center(0, 0, 0);
    ASSERT(! mesh.vertices.empty());
    const size_t n = mesh.vertices.size();
    for (size_t i = 0; i < n; ++i)
        center += mesh.vertices[i]->point;
    return center / n;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Dump3dv functions.
// ----------------------------------------------------------------------------

namespace Debug {

Dump3dv::Dump3dv(const FilePath &path, const std::string &header) {
    out_.open(path.ToNativeString());
    if (! out_) {
        std::cerr << "*** Dump3dv unable to open "
                  << path.ToNativeString() << " for writing\n";
        return;
    }

    out_ << "# " << header << "\n\n";

    label_flags_.SetAll(true);
    label_font_size_ = 20;
}

Dump3dv::~Dump3dv() {
    out_.close();
}

void Dump3dv::AddTriMesh(const TriMesh &mesh,
                         const TFaceHighlightFunc &face_highlight_func,
                         const TEdgeHighlightFunc &edge_highlight_func,
                         const TVertexHighlightFunc &vert_highlight_func) {
    // Shorthand.
    const auto &pts  = mesh.points;
    const auto &inds = mesh.indices;

    // Points.
    out_ << "\n# TriMesh with " << pts.size() << " points:\n";
    for (size_t i = 0; i < pts.size(); ++i)
        AddVertex_(IID_("V", i), pts[i]);

    // Triangles as edges.
    out_ << "\n#  " << mesh.GetTriangleCount()
         << " Triangles as edges:\n" << "c " << kEdgeColor << "\n";
    if (! edge_highlight_func) {
        for (size_t i = 0; i < inds.size(); i += 3)
            out_ << "l " << extra_prefix_ << "E" << (i / 3)
                 << " " << IID_("V", inds[i])
                 << " " << IID_("V", inds[i + 1])
                 << " " << IID_("V", inds[i + 2])
                 << " " << IID_("V", inds[i])
                 << "\n";
    }
    else {
        const auto add_edge = [&](size_t ind, char sub, GIndex v0, GIndex v1){
            const auto id = extra_prefix_ + "E" + Util::ToString(ind / 3) + sub;
            AddEdge(id, IID_("V", v0), IID_("V", v1));
        };
        for (size_t i = 0; i < inds.size(); i += 3) {
            if (! edge_highlight_func(inds[i], inds[i + 1]))
                add_edge(i, 'a', inds[i], inds[i + 1]);
            if (! edge_highlight_func(inds[i + 1], inds[i + 2]))
                add_edge(i, 'b', inds[i + 1], inds[i + 2]);
            if (! edge_highlight_func(inds[i + 2], inds[i]))
                add_edge(i, 'c', inds[i + 2], inds[i]);
        }
        // Highlighted edges.
        out_ << "c " << kHighlightEdgeColor << "\n";
        for (size_t i = 0; i < inds.size(); i += 3) {
            if (edge_highlight_func(inds[i], inds[i + 1]))
                add_edge(i, 'a', inds[i], inds[i + 1]);
            if (edge_highlight_func(inds[i + 1], inds[i + 2]))
                add_edge(i, 'b', inds[i + 1], inds[i + 2]);
            if (edge_highlight_func(inds[i + 2], inds[i]))
                add_edge(i, 'c', inds[i + 2], inds[i]);
        }
    }

    // Triangles as translucent faces.
    out_ << "\n#  Triangles as faces:\n";
    for (size_t i = 0; i < inds.size(); i += 3) {
        AltFaceColor_(i / 3);
        std::string vids;
        for (int j = 0; j < 3; ++j) {
            const int vi = inds[i + j];
            const std::string vid = IID_("F", i) + "_" + IID_("V", vi);
            AddVertex_(vid, pts[vi]);
            vids += " " + vid;
        }
        out_ << "f " << IID_("F", i / 3) << vids << "\n";
    }

    out_ << "\ns " << label_font_size_ << "\n\n";
    if (label_flags_.Has(LabelFlag::kVertexLabels)) {
        out_ << "\n# Vertex labels:\n" << "c " << kVertexLabelColor << "\n";
        for (size_t i = 0; i < pts.size(); ++i)
            if (! vert_highlight_func || ! vert_highlight_func(i))
                AddLabel_(pts[i], IID_("V", i));
        // Highlighted labels.
        if (vert_highlight_func) {
            out_ << "c " << kHighlightVertexLabelColor << "\n";
            for (size_t i = 0; i < pts.size(); ++i)
            if (vert_highlight_func(i))
                AddLabel_(pts[i], IID_("V", i));
        }
    }
    if (label_flags_.Has(LabelFlag::kFaceLabels)) {
        const Point3f mesh_center = ComputeMeshBounds(mesh).GetCenter();
        const auto do_face = [&](size_t ind){
            const Point3f tri_center = GetTriCenter_(mesh, ind);
            const Point3f pos = tri_center + .05f * (tri_center - mesh_center);
            AddRotatedLabel_(pos, GetTriNormal_(mesh, ind), IID_("F", ind));
        };
        out_ << "\n# Face labels:\n" << "c .8 .9 .8 1\n";
        for (size_t i = 0; i < inds.size() / 3; ++i) {
            if (! face_highlight_func || ! face_highlight_func(i))
                do_face(i);
        }
        // Highlighted labels.
        if (face_highlight_func) {
            out_ << "c " << kHighlightFaceLabelColor << "\n";
            for (size_t i = 0; i < inds.size() / 3; ++i) {
                if (face_highlight_func(i))
                    do_face(i);
            }
        }
    }
}

void Dump3dv::AddPolyMesh(const PolyMesh &mesh,
                          const PFaceHighlightFunc &face_highlight_func,
                          const PEdgeHighlightFunc &edge_highlight_func,
                          const PVertexHighlightFunc &vert_highlight_func) {
    // Vertices.
    out_ << "\n# PolyMesh with " << mesh.vertices.size() << " vertices:\n";
    for (const auto &v: mesh.vertices)
        AddVertex_(ID_(v->id), v->point);

    // Edges.
    out_ << "\n#  " << mesh.edges.size() << " edges:\n";
    out_ << "c " << kEdgeColor << "\n";
    for (const auto &e: mesh.edges)
        if (! edge_highlight_func || ! edge_highlight_func(*e))
            out_ << "l " << ID_(e->id)
                 << ' ' << ID_(e->v0->id)
                 << ' ' << ID_(e->v1->id) << "\n";
    // Highlighted edges.
    if (edge_highlight_func) {
        out_ << "c " << kHighlightEdgeColor << "\n";
        for (const auto &e: mesh.edges)
            if (edge_highlight_func(*e))
                out_ << "l " << ID_(e->id)
                     << ' ' << ID_(e->v0->id)
                     << ' ' << ID_(e->v1->id) << "\n";
    }

    // Translucent faces.
    out_ << "\n# " << mesh.faces.size() << " faces:\n";
    for (size_t i = 0; i < mesh.faces.size(); ++i) {
        const auto &f = *mesh.faces[i];
        // Skip faces that have been merged (in case this is called during
        // the merge operation).
        if (f.is_merged)
            continue;
        AltFaceColor_(i);
        std::string vids;
        auto add_face_verts = [&](const PolyMesh::EdgeVec &edges){
            for (const auto &e: edges) {
                const std::string vid = ID_(f.id) + "_" + ID_(e->v0->id);
                AddVertex_(vid, e->v0->point);
                vids += " " + vid;
            }
        };
        add_face_verts(f.outer_edges);
        for (size_t h = 0; h < f.hole_edges.size(); ++h) {
            // Semicolon separates contours in output.
            vids += " ;";
            add_face_verts(f.hole_edges[h]);
        }
        out_ << "f " << ID_(f.id) << vids << "\n";
    }

    out_ << "\ns " << label_font_size_ << "\n\n";
    if (label_flags_.Has(LabelFlag::kVertexLabels)) {
        out_ << "\n# Vertex labels:\n" << "c " << kVertexLabelColor << "\n";
        for (const auto &v: mesh.vertices)
            if (! vert_highlight_func || ! vert_highlight_func(*v))
                AddLabel_(v->point, ID_(v->id));
        // Highlighted labels.
        if (vert_highlight_func) {
            out_ << "c " << kHighlightVertexLabelColor << "\n";
            for (const auto &v: mesh.vertices)
                if (vert_highlight_func(*v))
                    AddLabel_(v->point, ID_(v->id));
        }
    }
    if (label_flags_.Has(LabelFlag::kEdgeLabels)) {
        out_ << "\n# Edge labels:\n" << "c " << kEdgeLabelColor << "\n";
        for (auto &e: mesh.edges)
            if (! edge_highlight_func || ! edge_highlight_func(*e))
                AddLabel_(GetEdgeLabelPos_(*e), ID_(e->id));
        // Highlighted labels.
        if (edge_highlight_func) {
            out_ << "c " << kHighlightEdgeLabelColor << "\n";
            for (auto &e: mesh.edges)
                if (edge_highlight_func(*e))
                    AddLabel_(GetEdgeLabelPos_(*e), ID_(e->id));
        }
    }
    if (label_flags_.Has(LabelFlag::kFaceLabels)) {
        const Point3f mesh_center = GetPolyMeshCenter_(mesh);
        out_ << "\n# Face labels:\n" << "c " << kFaceLabelColor << "\n";
        for (auto &f: mesh.faces) {
            if (f->is_merged ||
                (face_highlight_func && face_highlight_func(*f)))
                continue;
            const Point3f face_center = GetFaceCenter_(*f);
            const Point3f pos = face_center + .1f * (face_center - mesh_center);
            AddRotatedLabel_(pos, f->GetNormal(), ID_(f->id));
        }
        // Highlighted labels.
        if (face_highlight_func) {
            out_ << "c " << kHighlightFaceLabelColor << "\n";
            for (auto &f: mesh.faces)
                if (face_highlight_func(*f)) {
                    const Point3f face_center = GetFaceCenter_(*f);
                    const Point3f pos =
                        face_center + .1f * (face_center - mesh_center);
                    AddLabel_(pos, ID_(f->id));
                }
        }
    }
}

void Dump3dv::AddVertex(const std::string &id, const Point3f &point) {
    AddVertex_(id, point);
    if (label_flags_.Has(LabelFlag::kVertexLabels))
        AddLabel_(point, id);
}

void Dump3dv::AddEdge(const std::string &id,
                      const std::string &v0_id, const std::string &v1_id) {
    out_ << "l " << id << ' ' << v0_id << ' ' << v1_id << "\n";
}

void Dump3dv::AddFace(const std::string &id,
                      const std::vector<std::string> &vids) {
    AltFaceColor_(0);
    out_ << "f " << id << " " << Util::JoinItems(vids, " ") << "\n";
}

void Dump3dv::AddVertex_(const std::string &id, const Point3f &p) {
    out_ << "v " << id << p << "\n";
}

void Dump3dv::AddLabel_(const Point3f &pos, const std::string &text) {
    Point3f label_pos = pos + extra_label_offset_;

    // If there is already a label at this position, keep adding the offset.
    while (label_point_map_.Contains(label_pos))
        label_pos += coincident_label_offset_;
    label_point_map_.Add(label_pos);

    out_ << "t" << label_pos << ' ' << text << "\n";
}

void Dump3dv::AddRotatedLabel_(const Point3f &pos, const Vector3f &dir,
                               const std::string &text) {
    Point3f label_pos = pos + extra_label_offset_;

    // If there is already a label at this position, keep adding the offset.
    while (label_point_map_.Contains(label_pos))
        label_pos += coincident_label_offset_;
    label_point_map_.Add(label_pos);

    out_ << "T" << label_pos << ' ' << dir << ' ' << text << "\n";
}

void Dump3dv::AltFaceColor_(size_t i) {
    // Set up N colors spaced out in hue.
    static const int   kColorCount = 8;
    static const float kSat        = .4f;
    static const float kVal        = .95;
    static const float kAlpha      = .25f;
    static std::vector<Color> colors;

    // Populate colors if not already done.
    if (colors.empty()) {
        colors.reserve(kColorCount);
        for (int i = 0; i < kColorCount; ++i) {
            const float hue = static_cast<float>(i) / kColorCount;
            colors.push_back(Color::FromHSV(hue, kSat, kVal));
        }
        // Randomize the vector so that neighboring colors are not always
        // similar.
        auto rng = std::default_random_engine{};
        std::shuffle(colors.begin(), colors.end(), rng);
    }

    const Color &color = colors[i % kColorCount];
    out_ << "c " << color[0] << " " << color[1] << " " << color[2]
         << " " << kAlpha << "\n";
}

std::string Dump3dv::ID_(const std::string &id) {
    return extra_prefix_ + id;
}

std::string Dump3dv::IID_(const std::string &prefix, int index) {
    return ID_(prefix + Util::ToString(index));
}

}  // namespace Debug
