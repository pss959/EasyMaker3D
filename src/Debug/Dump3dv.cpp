#include "Debug/Dump3dv.h"

#include <fstream>

#include "Math/Linear.h"
#include "Math/MeshUtils.h"
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


/// Dumps a 3dv vertex statement.
static void DumpVertex_(std::ostream &out,
                        const std::string &id, const Point3f &p) {
    out << "v " << id << p << "\n";
}

/// Dumps a 3dv text statement for a label. Labels are pushed forward in Z a
/// little.
static void DumpLabel_(std::ostream &out,
                       const Point3f &pos, const std::string &text) {
    static const float kLabelZOffset = .01f;
    const Point3f label_pos(pos[0], pos[1], pos[2] + kLabelZOffset);
    out << "t" << label_pos << ' ' << text << "\n";
}

/// Creates an ID string from a prefix and index.
static std::string ID_(const std::string &prefix, int index) {
    return prefix + Util::ToString(index);
}

/// Outputs alternating colors for faces depending on an index.
static void AltFaceColors_(std::ostream &out, size_t i) {
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
    out << "c " << color[0] << " " << color[1] << " " << color[2]
        << " " << kAlpha << "\n";
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

void Dump3dv::DumpTriMesh(const TriMesh &mesh,
                          const std::string &description,
                          const FilePath &path,
                          const LabelFlags &label_flags) {
    std::ofstream out(path.ToNativeString());
    if (! out) {
        std::cerr << "*** Unable to open " << path.ToNativeString()
                  << " for DumpPolyMesh\n";
        return;
    }

    // Shorthand.
    const auto &pts  = mesh.points;
    const auto &inds = mesh.indices;

    // Points.
    out << "# " << description << " TriMesh with "
        << pts.size() << " points:\n";
    for (size_t i = 0; i < pts.size(); ++i)
        DumpVertex_(out, ID_("V", i), pts[i]);

    // Triangles as edges.
    out << "\n#  " << mesh.GetTriangleCount()
        << " Triangles as edges:\n" << "c 1 1 0 1\n";
    for (size_t i = 0; i < inds.size(); i += 3)
        out << "l E" << (i / 3)
            << " V" << inds[i]
            << " V" << inds[i + 1]
            << " V" << inds[i + 2]
            << " V" << inds[i]
            << "\n";

    // Triangles as translucent faces.
    out << "\n#  Triangles as faces:\n";
    for (size_t i = 0; i < inds.size(); i += 3) {
        const auto i0 = inds[i + 0];
        const auto i1 = inds[i + 1];
        const auto i2 = inds[i + 2];
        AltFaceColors_(out, i);
        const std::string vid0 = ID_("F", i) + "_" + ID_("V", i0);
        const std::string vid1 = ID_("F", i) + "_" + ID_("V", i1);
        const std::string vid2 = ID_("F", i) + "_" + ID_("V", i2);
        DumpVertex_(out, vid0, pts[i0]);
        DumpVertex_(out, vid1, pts[i1]);
        DumpVertex_(out, vid1, pts[i2]);
        out << "f " << ID_("F", i)
            << ' ' << vid0
            << ' ' << vid1
            << ' ' << vid2 << "\n";
    }

    if (label_flags.Has(LabelFlag::kVertexLabels)) {
        out << "\n# Vertex labels:\n" << "c 1 1 1 1\n";
        for (size_t i = 0; i < pts.size(); ++i)
            DumpLabel_(out, pts[i], ID_("V", i));
    }

    if (label_flags.Has(LabelFlag::kFaceLabels)) {
        out << "\n# Face labels:\n" << "c .8 .9 .8 1\n";
        const Point3f mesh_center = ComputeMeshBounds(mesh).GetCenter();
        for (size_t i = 0; i < inds.size() / 3; ++i) {
            const Point3f tri_center = GetTriCenter_(mesh, i);
            const Point3f pos = mesh_center + 1.2f * (tri_center - mesh_center);
            DumpLabel_(out, pos, ID_("F", i));
        }
    }
}

void Dump3dv::DumpPolyMesh(const PolyMesh &poly_mesh,
                           const std::string &description,
                           const FilePath &path,
                           const LabelFlags &label_flags) {
    std::ofstream out(path.ToNativeString());
    if (! out) {
        std::cerr << "*** Unable to open " << path.ToNativeString()
                  << " for DumpPolyMesh\n";
        return;
    }

    // Vertices.
    out << "# " << description << ": PolyMesh with "
        << poly_mesh.vertices.size() << " vertices:\n";
    for (const auto &v: poly_mesh.vertices)
        DumpVertex_(out, v->id, v->point);

    // Edges.
    out << "\n#  " << poly_mesh.edges.size() << " edges:\n" << "c 1 1 0 1\n";
    for (const auto &e:  poly_mesh.edges)
        out << "l " << e->id << ' ' << e->v0->id << ' ' << e->v1->id << "\n";

    // Translucent faces.
    out << "\n# " << poly_mesh.faces.size() << " faces:\n";
    for (size_t i = 0; i < poly_mesh.faces.size(); ++i) {
        const auto &f = *poly_mesh.faces[i];
        // Skip faces that have been merged (in case this is called during
        // the merge operation).
        if (f.is_merged)
            continue;
        AltFaceColors_(out, i);
        std::string vids;
        auto add_face_verts = [&](const PolyMesh::EdgeVec &edges){
            for (const auto &e: edges) {
                const std::string vid = f.id + "_" + e->v0->id;
                DumpVertex_(out, vid, e->v0->point);
                vids += " " + vid;
            }
        };
        add_face_verts(f.outer_edges);
        for (size_t h = 0; h < f.hole_edges.size(); ++h) {
            // Semicolon separates contours in output.
            vids += " ;";
            add_face_verts(f.hole_edges[h]);
        }
        out << "f " << f.id << vids << "\n";
    }

    out << "\ns 10\n\n";  // Smaller font size
    if (label_flags.Has(LabelFlag::kVertexLabels)) {
        out << "\n# Vertex labels:\n" << "c 1 1 1 1\n";
        for (const auto &v: poly_mesh.vertices)
            DumpLabel_(out, v->point, v->id);
    }
    if (label_flags.Has(LabelFlag::kEdgeLabels)) {
        out << "\n# Edge labels:\n" << "c 1 1 .5 1\n";
        for (auto &e: poly_mesh.edges)
            DumpLabel_(out, GetEdgeLabelPos_(*e), e->id);
    }
    if (label_flags.Has(LabelFlag::kFaceLabels)) {
        const Point3f mesh_center = GetPolyMeshCenter_(poly_mesh);
        out << "\n# Face labels:\n" << "c .8 .9 .8 1\n";
        for (auto &f: poly_mesh.faces) {
            if (f->is_merged)
                continue;
            const Point3f face_center = GetFaceCenter_(*f);
            const Point3f pos = face_center + .1f * (face_center - mesh_center);
            DumpLabel_(out, pos, f->id);
        }
    }
}

void Dump3dv::DumpEdges(const std::vector<Point3f> &vertices,
                        const std::vector<GIndex>  &edges,
                        const std::string &description,
                        const FilePath &path,
                        const LabelFlags &label_flags,
                        float label_font_size) {
    std::ofstream out(path.ToNativeString());
    if (! out) {
        std::cerr << "*** Unable to open " << path.ToNativeString()
                  << " for DumpEdges\n";
        return;
    }

    // Vertices.
    out << "# " << description << " with " << vertices.size() << " vertices:\n";
    for (size_t i = 0; i < vertices.size(); ++i)
        DumpVertex_(out, ID_("V", i), vertices[i]);

    // Edges.
    out << "\n#  " << (edges.size() / 2) << " edges:\n" << "c 1 1 0 1\n";
    for (size_t i = 0; i < edges.size(); i += 2)
        out << "l " << ID_("E", i / 2) << ' ' << ID_("V", edges[i])
            << ' ' << ID_("V", edges[i + 1]) << "\n";

    out << "\ns " << label_font_size << "\n\n";
    if (label_flags.Has(LabelFlag::kVertexLabels)) {
        out << "\n# Vertex labels:\n" << "c 1 1 1 1\n";
        for (size_t i = 0; i < vertices.size(); ++i)
            DumpLabel_(out, vertices[i], ID_("V", i));
    }
    if (label_flags.Has(LabelFlag::kEdgeLabels)) {
        out << "\n# Edge labels:\n" << "c 1 1 .5 1\n";
        for (size_t i = 0; i < edges.size(); i += 2) {
            const Point3f pos =
                .5f * (vertices[edges[i]] + vertices[edges[i + 1]]);
            DumpLabel_(out, pos, ID_("E", i / 2));
        }
    }
}

}  // namespace Debug
