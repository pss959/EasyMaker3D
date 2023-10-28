#include "Math/Triangulation.h"

#include "Math/CGALInternal.h"
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>

#include "Util/Assert.h"

namespace {

// ----------------------------------------------------------------------------
// The Triangulator class encapsulates triangulation functions.
// ----------------------------------------------------------------------------

class Triangulator_ {
  public:
    /// Triangulates a Polygon, possibly with holes.
    std::vector<GIndex> Triangulate(const Polygon &poly);

  private:
    /// Stores the location and origin of a vertex.
    struct Vertex_ {
        CPoint2 point;       // Location.
        int     hole_index;  // -1 for outer boundary, >= 0 for hole boundary.
        // Constructor assumes not a hole.
        Vertex_(const Point2f &p) : point(CPoint2(p[0], p[1])),
                                    hole_index(-1) {}
    };

    /// Stores info for a face to be able to mark domains.
    struct FaceInfo_ {
        int nesting_level;
        FaceInfo_() { nesting_level = -1; }

        // Returns true if the face is in the domain (after marking).
        bool IsInDomain() const {
            return nesting_level % 2 == 1;  // Nesting level is odd.
        }
    };

    // Shorthand.
    using CPoly2 = CGAL::Polygon_2<CKernel>;
    using CVB    = CGAL::Triangulation_vertex_base_2<CKernel>;
    using CFBB   = CGAL::Triangulation_face_base_with_info_2<FaceInfo_, CKernel>;
    using CFB    = CGAL::Constrained_triangulation_face_base_2<CKernel,CFBB>;
    using CTDS   = CGAL::Triangulation_data_structure_2<CVB,CFB>;
    using CItag  = CGAL::Exact_predicates_tag;
    using CCDT   = CGAL::Constrained_Delaunay_triangulation_2<CKernel, CTDS,
                                                              CItag>;
    using CVH    = CCDT::Vertex_handle;
    using CFH    = CCDT::Face_handle;

    /// All vertices of the polygon to triangulate.
    std::vector<Vertex_> vertices_;

    /// Maps Vertex_handle (CVH) to index in vector of Vertex_ instances.
    using VMap_ = std::unordered_map<CVH, GIndex>;
    VMap_ vmap_;

    /// Stores a Vertex_ instance for each point in the given Polygon.
    void StoreVertices_(const Polygon &poly);

    void AddBorderToTriangulation_(size_t start_index, size_t count,
                                   CCDT &triangulation);

    /// Returns the indices for all result triangles in counterclockwise order.
    std::vector<GIndex> GetTriangleIndices_(CCDT &triangulation);

    /// Marks domains for a triangulation. Stolen from CGAL doc.
    static void MarkDomains_(CCDT &triangulation);
    static void MarkDomains_(CCDT &triangulation, CFH start, int index,
                             std::list<CCDT::Edge> &border);
};

std::vector<GIndex> Triangulator_::Triangulate(const Polygon &poly) {
    // Store a _Vertex instance for each point.
    StoreVertices_(poly);

    // Create a triangulation and add each border to it.
    CCDT triangulation;
    size_t vert_index = 0;
    const std::vector<size_t> &border_counts = poly.GetBorderCounts();
    for (size_t b = 0; b < border_counts.size(); ++b) {
        const size_t count = border_counts[b];
        AddBorderToTriangulation_(vert_index, count, triangulation);
        vert_index += count;
    }

    // Mark facets that are inside the domain bounded by the polygon.
    MarkDomains_(triangulation);

    // Collect triangle indices.
    return GetTriangleIndices_(triangulation);
}

void Triangulator_::StoreVertices_(const Polygon &poly) {
    // Create a Vertex_ for each point.
    const std::vector<Point2f> &points = poly.GetPoints();
    vertices_.reserve(points.size());
    for (const auto &p: points)
        vertices_.push_back(Vertex_(p));
    ASSERT(vertices_.size() == points.size());

    // Update hole indices in each Vertex_ if there are any holes.
    size_t vert_index = 0;
    int    hole_index = -1;
    const std::vector<size_t> &border_counts = poly.GetBorderCounts();
    for (size_t b = 0; b < border_counts.size(); ++b) {
        for (size_t i = 0; i < border_counts[b]; ++i)
            vertices_[vert_index++].hole_index = hole_index;
        ++hole_index;
    }
    ASSERT(vert_index <= points.size());
}

void Triangulator_::AddBorderToTriangulation_(size_t start_index, size_t count,
                                              CCDT &triangulation) {
    // Store vertex handles so constraint edges can be added.
    std::vector<CVH> vh(count);

    for (size_t i = 0; i < count; ++i) {
        const size_t vert_index = start_index + i;
        vh[i] = triangulation.insert(vertices_[vert_index].point);
        vmap_[vh[i]] = vert_index;
        if (i > 0)
            triangulation.insert_constraint(vh[i - 1], vh[i]);
    }
    // Connect last vertex to first.
    triangulation.insert_constraint(vh[count - 1], vh[0]);
}

std::vector<GIndex> Triangulator_::GetTriangleIndices_(CCDT &triangulation) {
    std::vector<GIndex> indices;
    for (auto face: triangulation.finite_face_handles()) {
        // Skip faces that are not marked as part of the domain.
        if (face->info().IsInDomain()) {
            indices.push_back(vmap_[face->vertex(0)]);
            indices.push_back(vmap_[face->vertex(1)]);
            indices.push_back(vmap_[face->vertex(2)]);
        }
    }
    return indices;
}

void Triangulator_::MarkDomains_(CCDT &triangulation) {
    for (CFH f: triangulation.all_face_handles())
        f->info().nesting_level = -1;

    std::list<CCDT::Edge> border;
    MarkDomains_(triangulation, triangulation.infinite_face(), 0, border);
    while (! border.empty()) {
        CCDT::Edge e = border.front();
        border.pop_front();
        CFH n = e.first->neighbor(e.second);
        if (n->info().nesting_level == -1)
            MarkDomains_(triangulation, n,
                         e.first->info().nesting_level + 1, border);
    }
}

void Triangulator_::MarkDomains_(CCDT &triangulation, CFH start, int index,
                                 std::list<CCDT::Edge> &border) {
    if (start->info().nesting_level != -1)
        return;

    std::list<CFH> queue;
    queue.push_back(start);
    while (! queue.empty()){
        CFH fh = queue.front();
        queue.pop_front();
        if (fh->info().nesting_level == -1) {
            fh->info().nesting_level = index;
            for (int i = 0; i < 3; i++) {
                CCDT::Edge e(fh, i);
                CFH n = fh->neighbor(i);
                if (n->info().nesting_level == -1) {
                    if (triangulation.is_constrained(e))
                        border.push_back(e);
                    else
                        queue.push_back(n);
                }
            }
        }
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

std::vector<GIndex> TriangulatePolygon(const Polygon &poly) {
    return Triangulator_().Triangulate(poly);
}
