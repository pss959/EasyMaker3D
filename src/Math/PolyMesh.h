#pragma once

#include "Math/Types.h"
#include "Util/String.h"

/// A PolyMesh represents a solid, watertight mesh constructed from polygonal
/// faces, possibly with holes. It has vertex, face, and edge connectivity
/// information that allows operations to be performed on it. It is convertible
/// to and from a TriMesh.
///
/// There are three main nested structs: Vertex, Edge, and Face.  A PolyMesh is
/// defined as a collection of Face instances. Each Face is defined by a list
/// of Edge instances. Each Edge has a Vertex at either end. Each Vertex
/// instance should be unique, allowing the resulting mesh to be watertight.
///
/// The Vertex, Edge, and Face structs are derived from the Feature struct,
/// which stores a string identifier. All IDs are unique for each type of
/// feature. A Vertex will have an ID of the form "Vn", where n is incremented
/// for each Vertex created. Edge IDs are "En", and Face IDs are "Fn".
///
/// Note that all storage and accessors are non-const for simplicity.
///
/// The PolyMeshBuilder struct can be used to construct a PolyMesh
/// incrementally.
///
/// \ingroup Math
struct PolyMesh {
    // Forward references.
    struct Edge;
    struct Face;
    struct Feature;
    struct Vertex;
    struct Border;

    /// Convenience alias for a vector of edge pointers.
    using EdgeVec = std::vector<Edge *>;

    /// Convenience alias for a vector of face pointers.
    using FaceVec = std::vector<Face *>;

    /// Convenience alias for a vector of vertex pointers.
    using VertexVec = std::vector<Vertex *>;

    /// Convenience alias for a vector of GIndex indices.
    using IndexVec = std::vector<GIndex>;

    /// All vertices in the PolyMesh.
    VertexVec vertices;

    /// All faces in the PolyMesh.
    FaceVec   faces;

    /// All edges in the PolyMesh.
    EdgeVec   edges;

    /// Constructs an empty PolyMesh.
    PolyMesh() {}

    /// Constructs a PolyMesh from a watertight TriMesh. This assumes all
    /// vertices in the mesh are unique and shared.
    PolyMesh(const TriMesh &mesh);

    /// Builds a PolyMesh from vertex points and borders.
    PolyMesh(const std::vector<Point3f> &points,
             const std::vector<Border> &borders) {
        Set(points, borders);
    }

    ~PolyMesh() { Clear(); }

    /// Replaces the contents using the given vertex points and borders.
    void Set(const std::vector<Point3f> &points,
             const std::vector<Border> &borders);

    /// Clears and deletes the current contents, if any.
    void Clear();

    /// Changes all ID's in vertices, edges, and faces to be consecutive.
    void ReindexIDs();

    // Collects vertices from all borders of the given Face into the vertices
    // vector. The border_counts vector will contain the number of vertices in
    // each border (outside, followed by holes, if any).
    static void GetFaceVertices(const Face &face, VertexVec &vertices,
                                std::vector<size_t> &border_counts);

    /// Converts the PolyMesh to a TriMesh and returns it.
    TriMesh ToTriMesh() const;

    /// Dumps the PolyMesh to stdout for debugging.
    void Dump(const Str &when) const;

    /// Do not allow copy construction or assignment; pointers would be wrong.
    PolyMesh(const PolyMesh &)             = delete;
    PolyMesh & operator=(const PolyMesh &) = delete;
};

// ----------------------------------------------------------------------------
// PolyMesh::Feature struct definition.
// ----------------------------------------------------------------------------

/// Base for Vertex, Edge, and Face. It stores a unique string ID.
struct PolyMesh::Feature {
    /// String identifier for the feature. This is set at constructon.
    Str id;
  protected:
    Feature(const Str &prefix, int index);
};

// ----------------------------------------------------------------------------
// PolyMesh::Vertex struct definition.
// ----------------------------------------------------------------------------

/// A vertex of the mesh.
struct PolyMesh::Vertex : public PolyMesh::Feature {
    /// The point defining the vertex position.
    Point3f point{0, 0, 0};

    /// Constructs a Vertex with the given ID and position.
    Vertex(int id, const Point3f &p) : Feature("V", id) { point = p; }

    /// Converts to a string for debugging.
    Str ToString() const;
};

// ----------------------------------------------------------------------------
// PolyMesh::Edge struct definition.
// ----------------------------------------------------------------------------

/// An Edge represents a directed edge between two vertices that separates two
/// faces. The face the edge is part of is to the left of the edge when
/// traveling from v0 to v1.
struct PolyMesh::Edge : public PolyMesh::Feature {
    Vertex *v0;               ///< Vertex at the start of the edge.
    Vertex *v1;               ///< Vertex at the end of the edge.
    Face   *face;             ///< The face the edge is part of.
    int     face_hole_index;  ///< Index within hole; -1 if an outer edge.
    int     index_in_face;    ///< Index within the Face's list of edges.
    Edge   *opposite_edge;    ///< Opposite edge; initially null.

    /// Constructs an edge with the given data.
    Edge(int id, Vertex &v0, Vertex &v1, Face &face, int face_hole_index);

    /// Returns a unit vector from v0 to v1.
    Vector3f GetUnitVector() const;

    /// Returns true if the edge forms part of a hole in a face.
    bool IsOnHoleBorder() const { return face_hole_index >= 0; }

    /// Returns true if the edge is vestigial, meaning that it has the same
    /// face on both sides.
    bool IsVestigial() const { return face == opposite_edge->face; }

    /// Convenience that returns the Edge after this one in its face, wrapping
    /// around if necessary. Note that this stays on the current border,
    /// whether it is the outside or a hole.
    Edge & NextEdgeInFace() const;

    /// Convenience that returns the Edge before this one in its face,
    /// wrapping around if necessary. Note that this stays on the current
    /// border, whether it is the outside or a hole.
    Edge & PreviousEdgeInFace() const;

    /// Convenience that returns the Edge counterclockwise from this one
    /// incident to the v0 vertex.
    Edge & NextEdgeAroundVertex() const;

    /// Convenience that returns the Edge clockwise from this one incident to
    /// the v0 vertex.
    Edge & PreviousEdgeAroundVertex() const;

    /// Converts to a string for debugging.
    Str ToString() const;
};

// ----------------------------------------------------------------------------
// PolyMesh::Face struct definition.
// ----------------------------------------------------------------------------

/// A polygonal face containing 3 or more vertices/edges. There is one outer
/// border of edges and N optional hole borders.
struct PolyMesh::Face : public PolyMesh::Feature {
    /// Ordered vector of directed edges defining the outer border of the face,
    /// traveling counterclockwise when viewed from outside the face.
    EdgeVec outer_edges;

    /// Vector of vectors of directed edges forming holes in the face,
    /// traveling clockwise. This is typically empty.
    std::vector<EdgeVec> hole_edges;

    /// Flag indicating whether this face was merged into another. This is used
    /// primarily for debugging.
    bool is_merged = false;

    /// Constructs a Face with the given ID.
    Face(int id) : Feature("F", id) {}

    /// Returns the normal to the face.
    const Vector3f & GetNormal() const;

    /// Returns the area of the outer border of the Face. (Holes are ignored.)
    float GetOuterArea() const;

    /// Returns the number of holes in the face, which is typically 0.
    size_t GetHoleCount() const { return hole_edges.size(); }

    /// Replaces one outer edge of the Face with a different edge.
    void ReplaceEdge(Edge &old_edge, Edge &new_edge);

    /// Reindexes all edges in the face.
    void ReindexEdges();

    /// Converts to a string for debugging.
    Str ToString(bool on_one_line = true) const;

  private:
    /// Normal to the face, computed only when necessary.
    mutable Vector3f normal_;

    friend struct PolyMesh;
};

// ----------------------------------------------------------------------------
// PolyMesh::Border struct definition.
// ----------------------------------------------------------------------------

/// A Border contains indices of the vertices that form border edges. It is
/// assumed to be closed. The is_hole flag indicates whether this is an outer
/// border or a hole border.
struct PolyMesh::Border {
    IndexVec indices;  ///< Indices of vertices forming the border.
    bool     is_hole;  ///< Whether this is a hole border or outer border.
};
