#include "Debug/Dump3dv.h"

#include <fstream>

#include "Math/Linear.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

//! Outputs a Point3f with rounded precision.
inline std::ostream & operator<<(std::ostream &out, const Point3f &p) {
    for (int i = 0; i < 3; ++i)
        out << ' ' << RoundToPrecision(p[i], .01f);
    return out;
}


//! Dumps a 3dv vertex statement.
static void DumpVertex_(std::ostream &out,
                        const std::string &id, const Point3f &p) {
    out << "v " << id << p << "\n";
}

//! Dumps a 3dv text statement for a label.
static void DumpLabel_(std::ostream &out,
                       const Point3f &pos, const std::string &text) {
    out << "t" << pos << ' ' << text << "\n";
}

//! Creates an ID string from a prefix and index.
static std::string ID_(const std::string &prefix, int index) {
    return prefix + Util::ToString(index);
}

//! Outputs alternate colors depending on an index.
static void AltColors_(std::ostream &out, size_t i) {
    out << "c " << ((i & 1) == 0 ? ".6 .8 1" : "1 .6 .6") << "\n";
}

//! Returns the center point of a PolyMesh::Face.
static Point3f GetFaceCenter_(const PolyMesh::Face &face) {
    Point3f center(0, 0, 0);
    for (auto &edge: face.outer_edges)
        center += edge->v0->point;
    center /= face.outer_edges.size();
    return center;
}

//! Returns the position of a label for a PolyMesh::Edge.
static Point3f GetEdgeLabelPos_(const PolyMesh::Edge &edge) {
    return edge.v0->point + .3f * (edge.v1->point - edge.v0->point);
}

// ----------------------------------------------------------------------------
// Dump3dv functions.
// ----------------------------------------------------------------------------

void Dump3dv::DumpTriMesh(const TriMesh &mesh,
                          const std::string &description,
                          const Util::FilePath &path,
                          float face_shrinkage, bool add_labels) {
    std::ofstream out(path);
    if (! out) {
        std::cerr << "*** Unable to open " << path.ToString()
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
        << " Triangles as edges:\nc 1 1 0\n";
    for (size_t i = 0; i < inds.size(); i += 3)
        out << "l E" << (i / 3)
            << " V" << inds[i]
            << " V" << inds[i + 1]
            << " V" << inds[i + 2]
            << " V" << inds[i]
            << "\n";

    // Shrunken triangles as faces.
    if (face_shrinkage >= 0) {
        out << "\n#  Triangles as faces:\n";
        for (size_t i = 0; i < inds.size(); i += 3) {
            Point3f p0 = pts[inds[i]];
            Point3f p1 = pts[inds[i + 1]];
            Point3f p2 = pts[inds[i + 2]];
            const Point3f center = (p0 + p1 + p2) / 3;
            p0 = Lerp(face_shrinkage, p0, center);
            p1 = Lerp(face_shrinkage, p1, center);
            p2 = Lerp(face_shrinkage, p2, center);
            AltColors_(out, i);
            const std::string vid0 = ID_("F", i) + "_" + ID_("V", inds[i]);
            const std::string vid1 = ID_("F", i) + "_" + ID_("V", inds[i + 1]);
            const std::string vid2 = ID_("F", i) + "_" + ID_("V", inds[i + 2]);
            DumpVertex_(out, vid0, p0);
            DumpVertex_(out, vid1, p1);
            DumpVertex_(out, vid2, p2);
            out << "f " << ID_("F", i)
                << ' ' << vid0
                << ' ' << vid1
                << ' ' << vid2 << "\n";
        }
    }

    if (add_labels) {
        // Vertex labels.
        out << "\n# Vertex labels:\nc 1 1 1\n";
        for (size_t i = 0; i < pts.size(); ++i)
            DumpLabel_(out, pts[i], ID_("V", i));
    }
}

void Dump3dv::DumpPolyMesh(const PolyMesh &poly_mesh,
                           const std::string &description,
                           const Util::FilePath &path,
                           float face_shrinkage, int label_flags) {
    std::ofstream out(path);
    if (! out) {
        std::cerr << "*** Unable to open " << path.ToString()
                  << " for DumpPolyMesh\n";
        return;
    }

    // Vertices.
    out << "# " << description << ": PolyMesh with "
        << poly_mesh.vertices.size() << " vertices:\n";
    for (const auto &v: poly_mesh.vertices)
        DumpVertex_(out, v->id, v->point);

    // Edges.
    out << "\n#  " << poly_mesh.edges.size() << " edges:\nc 1 1 0\n";
    for (const auto &e:  poly_mesh.edges)
        out << "l " << e->id << ' ' << e->v0->id << ' ' << e->v1->id << "\n";

    // Shrunken faces.
    if (face_shrinkage >= 0) {
        out << "\n# " << poly_mesh.faces.size() << " faces:\n";
        for (size_t i = 0; i < poly_mesh.faces.size(); ++i) {
            const auto &f = *poly_mesh.faces[i];
            // Skip faces with holes: they would be drawn incorrectly.
            if (! f.hole_edges.empty()) {
                out << "# Skipping face " << f.id << " with holes:\n";
                out << "#   Outside:";
                for (auto &e: f.outer_edges)
                    out << " " << e->v0->id;
                out << "\n";
                for (size_t h = 0; h < f.hole_edges.size(); ++h) {
                    out << "#   Hole " << h << ":";
                    for (auto &e: f.hole_edges[h])
                        out << " " << e->v0->id;
                    out << "\n";
                }
            }
            else {
                AltColors_(out, i);
                std::string vids;
                const Point3f center = GetFaceCenter_(f);
                for (auto &e: f.outer_edges) {
                    const Point3f p =
                        Lerp(e->v0->point, center, face_shrinkage);
                    const std::string vid = f.id + "_" + e->v0->id;
                    DumpVertex_(out, vid, p);
                    vids += " " + vid;
                }
                out << "f " << f.id << vids << "\n";
            }
        }
    }

    if (label_flags & LabelFlag::kVertexLabels) {
        out << "\n# Vertex labels:\nc 1 1 1\n";
        for (const auto &v: poly_mesh.vertices)
            DumpLabel_(out, v->point + Vector3f(0, 0, .1), v->id);
    }
    if (label_flags & LabelFlag::kEdgeLabels) {
        out << "\n# Edge labels:\nc 1 1 .5\n";
        for (auto &e: poly_mesh.edges)
            DumpLabel_(out, GetEdgeLabelPos_(*e), e->id);
    }
    if (label_flags & LabelFlag::kFaceLabels) {
        out << "\n# Face labels:\nc .2 .5 .2\n";
        for (auto &f: poly_mesh.faces)
            DumpLabel_(out, GetFaceCenter_(*f) + Vector3f(0, 0, .01f), f->id);
    }
}

#if XXXX

    //! Dumps a Polygon to the log for debugging.
    public static void DumpPolygon(Polygon poly, string when,
                                   bool addLabels = true) {
        // Points.
        List<Vector2> points = poly.GetPoints();
        Debug.Log($"# {when}: Polygon with {points.Count} points:");
        for (int i = 0; i < points.Count; ++i)
            DumpVertex($"V{i}", points[i]);

        // Lines connecting border points.
            Debug.Log("");
        Debug.Log("c 1 1 0");
        List<uint> borderCounts = poly.GetBorderCounts();
        uint startIndex = 0;
        for (int i = 0; i < borderCounts.Count; ++i) {
            string line = $"l Border_{i} ";
            for (int j = 0; j < borderCounts[i]; ++j)
                line += $" V{startIndex + j}";
            line += $" V{startIndex}";  // Connect back to start.
            Debug.Log(line);
            startIndex += borderCounts[i];
        }

        if (addLabels) {
            Debug.Log("");
            Debug.Log("# Vertex labels:");
            Debug.Log("c 1 1 1");
            for (int i = 0; i < points.Count; ++i)
                DumpLabel(points[i], $"V{i}");
        }
    }

    // ------------------------------------------------------------------------
    // Implementation.
    // ------------------------------------------------------------------------

}

#endif
