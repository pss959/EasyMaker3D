#include "Debug/Dump3dv.h"

#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Math/PolyMesh.h"
#include "Math/Skeleton3D.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// Outputs a Point3f with rounded precision.
inline std::ostream & operator<<(std::ostream &out, const Point3f &p) {
    for (int i = 0; i < 3; ++i)
        out << ' ' << RoundToPrecision(p[i], .01f);
    return out;
}


/// Returns the center point of a TriMesh triangle.
static Point3f GetTriCenter_(const TriMesh &mesh, size_t tri_index) {
    return (mesh.points[mesh.indices[3 * tri_index + 0]] +
            mesh.points[mesh.indices[3 * tri_index + 1]] +
            mesh.points[mesh.indices[3 * tri_index + 2]]) / 3.f;
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

void Dump3dv::AddTriMesh(const TriMesh &mesh) {
    // Shorthand.
    const auto &pts  = mesh.points;
    const auto &inds = mesh.indices;

    // Points.
    out_ << "\n# TriMesh with " << pts.size() << " points:\n";
    for (size_t i = 0; i < pts.size(); ++i)
        AddVertex_(IID_("V", i), pts[i]);

    // Triangles as edges.
    out_ << "\n#  " << mesh.GetTriangleCount()
        << " Triangles as edges:\n" << "c 1 1 0 1\n";
    for (size_t i = 0; i < inds.size(); i += 3)
        out_ << "l " << extra_prefix_ << "E" << (i / 3)
             << " " << IID_("V", inds[i])
             << " " << IID_("V", inds[i + 1])
             << " " << IID_("V", inds[i + 2])
             << " " << IID_("V", inds[i])
             << "\n";

    // Triangles as translucent faces.
    out_ << "\n#  Triangles as faces:\n";
    for (size_t i = 0; i < inds.size(); i += 3) {
        AltFaceColor_(i);
        std::string vids;
        for (int j = 0; j < 3; ++j) {
            const int vi = inds[i + j];
            const std::string vid = IID_("F", i) + "_" + IID_("V", vi);
            AddVertex_(vid, pts[vi]);
            vids += " " + vid;
        }
        out_ << "f " << IID_("F", i) << vids << "\n";
    }

    out_ << "\ns " << label_font_size_ << "\n\n";
    if (label_flags_.Has(LabelFlag::kVertexLabels)) {
        out_ << "\n# Vertex labels:\n" << "c 1 1 1 1\n";
        for (size_t i = 0; i < pts.size(); ++i)
            AddLabel_(pts[i], IID_("V", i));
    }
    if (label_flags_.Has(LabelFlag::kFaceLabels)) {
        out_ << "\n# Face labels:\n" << "c .8 .9 .8 1\n";
        const Point3f mesh_center = ComputeMeshBounds(mesh).GetCenter();
        for (size_t i = 0; i < inds.size() / 3; ++i) {
            const Point3f tri_center = GetTriCenter_(mesh, i);
            const Point3f pos = mesh_center + 1.2f * (tri_center - mesh_center);
            AddLabel_(pos, IID_("F", i));
        }
    }
}

void Dump3dv::AddPolyMesh(const PolyMesh &mesh) {
    // Vertices.
    out_ << "\n# PolyMesh with " << mesh.vertices.size() << " vertices:\n";
    for (const auto &v: mesh.vertices)
        AddVertex_(ID_(v->id), v->point);

    // Edges.
    out_ << "\n#  " << mesh.edges.size() << " edges:\n" << "c 1 1 0 1\n";
    for (const auto &e: mesh.edges)
        out_ << "l " << ID_(e->id)
             << ' ' << ID_(e->v0->id)
             << ' ' << ID_(e->v1->id) << "\n";

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
        out_ << "\n# Vertex labels:\n" << "c 1 1 1 1\n";
        for (const auto &v: mesh.vertices)
            AddLabel_(v->point, ID_(v->id));
    }
    if (label_flags_.Has(LabelFlag::kEdgeLabels)) {
        out_ << "\n# Edge labels:\n" << "c 1 1 .5 1\n";
        for (auto &e: mesh.edges)
            AddLabel_(GetEdgeLabelPos_(*e), ID_(e->id));
    }
    if (label_flags_.Has(LabelFlag::kFaceLabels)) {
        const Point3f mesh_center = GetPolyMeshCenter_(mesh);
        out_ << "\n# Face labels:\n" << "c .8 .9 .8 1\n";
        for (auto &f: mesh.faces) {
            if (f->is_merged)
                continue;
            const Point3f face_center = GetFaceCenter_(*f);
            const Point3f pos = face_center + .1f * (face_center - mesh_center);
            AddLabel_(pos, ID_(f->id));
        }
    }
}

void Dump3dv::AddSkeleton3D(const Skeleton3D &skel) {
    const auto &vertices  = skel.GetVertices();
    const auto &edges     = skel.GetEdges();

    // Building a vertex label, which includes the source index and distance.
    const auto vlabel = [&](size_t i){
        const auto &v = vertices[i];
        std::string label = IID_("V", i) + "(";
        if (v.source_index >= 0)
            label += "s" + Util::ToString(v.source_index) + "/";
        label += "d" + Util::ToString(v.distance) + ")";
        return label;
    };

    // Building an edge label, which includes the bisected vertices.
    const auto elabel = [&](size_t i){
        const auto &e = edges[i];
        std::string label = IID_("E", i);
        if (e.bisected_index0 >= 0)
            label += "(" + Util::ToString(e.bisected_index0) + "/" +
                Util::ToString(e.bisected_index1) + ")";
        return label;
    };

    // Vertices.
    out_ << "\n# Skeleton3D with " << vertices.size() << " vertices:\n";
    for (size_t i = 0; i < vertices.size(); ++i)
        AddVertex_(IID_("V", i), vertices[i].point);

    // Edges.
    out_ << "\n#  " << edges.size() << " edges:\n" << "c .4 1 .9 1\n";
    for (size_t i = 0; i < edges.size(); ++i)
        out_ << "l " << IID_("E", i) << ' ' << IID_("V", edges[i].v0_index)
             << ' ' << IID_("V", edges[i].v1_index) << "\n";

    // Labels
    out_ << "\ns " << label_font_size_ << "\n\n";
    if (label_flags_.Has(LabelFlag::kVertexLabels)) {
        out_ << "\n# Vertex labels:\n" << "c .9 .6 .9 1\n";
        for (size_t i = 0; i < vertices.size(); ++i)
            AddLabel_(vertices[i].point, vlabel(i));
    }
    if (label_flags_.Has(LabelFlag::kEdgeLabels)) {
        out_ << "\n# Edge labels:\n" << "c .4 1 .9 1\n";
        for (size_t i = 0; i < edges.size(); ++i) {
            const auto &p0 = vertices[edges[i].v0_index].point;
            const auto &p1 = vertices[edges[i].v1_index].point;
            AddLabel_((p0 + p1) / 2, elabel(i));
        }
    }
}

void Dump3dv::AddVertex_(const std::string &id, const Point3f &p) {
    out_ << "v " << id << p << "\n";
}

void Dump3dv::AddLabel_(const Point3f &pos, const std::string &text) {
    static const float kLabelZOffset = .01f;
    const Point3f label_pos(pos[0], pos[1], pos[2] + kLabelZOffset);
    out_ << "t" << (label_offset_ + label_pos) << ' ' << text << "\n";
}

void Dump3dv::AltFaceColor_(size_t i) {
    // Set up 6 colors spaced out in hue.
    static const float kSat   = .4f;
    static const float kVal   = .95;
    static const float kAlpha = .25f;
    static const Color colors[6]{
        Color::FromHSV(0.f / 6, kSat, kVal),
        Color::FromHSV(2.f / 6, kSat, kVal),
        Color::FromHSV(4.f / 6, kSat, kVal),
        Color::FromHSV(1.f / 6, kSat, kVal),
        Color::FromHSV(3.f / 6, kSat, kVal),
        Color::FromHSV(5.f / 6, kSat, kVal),
    };

    const Color &color = colors[i % 6];
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
