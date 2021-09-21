#include <string>
#include <unordered_map>

#include "Math/Types.h"
#include "Util/String.h"

//! A PolyMesh represents a solid, watertight mesh constructed from polygonal
//! faces, possibly with holes. It has vertex, face, and edge connectivity
//! information that allows operations to be performed on it. It is convertible
//! to and from a TriMesh.
//!
//! There are three main nested classes: Vertex, Edge, and Face.  A PolyMesh is
//! defined as a collection of Face instances. Each Face is defined by a list
//! of Edge instances. Each Edge has a Vertex at either end. Each Vertex
//! instance should be unique, allowing the resulting mesh to be watertight.
//!
//! The Vertex, Edge, and Face classes are derived from the Feature class,
//! which stores a string identifier. All IDs are unique for each type of
//! feature. A Vertex will have an ID of the form "Vn", where n is incremented
//! for each Vertex created. Edge IDs are "En", and Face IDs are "Fn".
//!
//! The PolyMeshBuilder class can be used to construct a PolyMesh
//! incrementally.
//!
//! \ingroup Math
class PolyMesh {
  public:
    // Forward references.
    class Edge;
    class Face;
    class Feature;
    class Vertex;

    //! Convenience typedef for a vector of edge pointers.
    typedef std::vector<Edge *> EdgeVec;

    //! Constructs a PolyMesh from a watertight TriMesh. This assumes all
    //! vertices in the mesh are unique and shared.
    PolyMesh(const TriMesh &mesh);

#if XXXX
    //! Builds a PolyMesh from the contents of a PolyMeshBuilder.
    PolyMesh(const PolyMeshBuilder &builder);
#endif

    //! Returns a vector of all vertices in the PolyMesh.
    const std::vector<Vertex> & GetVertices() const { return vertices_; }
    //! Returns a vector of all faces in the PolyMesh.
    const std::vector<Face>   & GetFaces()    const { return faces_; }
    //! Returns a vector of all edges in the PolyMesh.
    const std::vector<Edge>   & GetEdges()    const { return edges_; }

    //! Convenience that returns the Edge after the given one in its face,
    //! wrapping around if necessary. Note that this stays on the current
    //! border, whether it is the outside or a hole.
    static Edge & NextEdgeInFace(const Edge &edge);

    //! Convenience that returns the Edge before the given one in its face,
    //! wrapping around if necessary. Note that this stays on the current
    //! border, whether it is the outside or a hole.
    static Edge & PreviousEdgeInFace(const Edge &edge);

    //! Returns a vector containing all edges meeting at the v0 vertex of the
    //! given edge, starting with the given edge.
    static EdgeVec GetVertexEdges(Edge &start_edge);

    //! Converts the PolyMesh to a TriMesh and returns it.
    TriMesh ToTriMesh() const;

  private:
    //! Maps an Edge ID to an Edge instance.
    typedef std::unordered_map<std::string, Edge *> EdgeMap_;

    std::vector<Vertex> vertices_;
    std::vector<Face>   faces_;
    std::vector<Edge>   edges_;

    //! Adds an edge between the given vertices.
    void AddEdge_(Face &face, Vertex &v0, Vertex &v1, EdgeMap_ &edge_map,
                  int hole_index = -1);

    //! Returns a unique hash key for an edge between two vertices.
    static std::string EdgeHashKey_(const Vertex &v0, const Vertex &v1);
};

// ----------------------------------------------------------------------------
// PolyMesh::Feature class definition.
// ----------------------------------------------------------------------------

//! Base class for Vertex, Edge, and Face. It stores a unique string ID.
class PolyMesh::Feature {
  public:
    const std::string & ID() const { return id_; }
  protected:
    Feature(const std::string &prefix, int id) :
        id_(prefix + Util::ToString(id)) {}
  private:
    const std::string id_;
};

// ----------------------------------------------------------------------------
// PolyMesh::Vertex class definition.
// ----------------------------------------------------------------------------

//! A vertex of the mesh.
class PolyMesh::Vertex : public PolyMesh::Feature {
  public:
    //! Constructs a Vertex with the given ID and position.
    Vertex(int id, const Point3f &p) : Feature("V", id) { point_ = p; }

    //! Returns the point defining the vertex position.
    const Point3f & GetPoint() const { return point_; }

  private:
    Point3f point_{0, 0, 0};
};

// ----------------------------------------------------------------------------
// PolyMesh::Edge class definition.
// ----------------------------------------------------------------------------

//! An Edge represents a directed edge between two vertices that separates two
//! faces. The face the edge is part of is to the left of the edge when
//! traveling from v0 to v1.
class PolyMesh::Edge : public PolyMesh::Feature {
  public:
    //! Constructs an edge with the given data.
    Edge(int id, Vertex &v0, Vertex &v1, Face &face,
         int face_hole_index, int index_in_face) :
        Feature("E", id), v0_(v0), v1_(v1), face_(face),
        face_hole_index_(face_hole_index), index_in_face_(index_in_face) {
    }

    //! Returns the Vertex at the start of the edge.
    const Vertex & GetV0() const { return v0_; }

    //! Returns the Vertex at the end of the edge.
    const Vertex & GetV1() const { return v1_; }

    //! Returns the Face the edge is part of.
    const Face & GetFace() const { return face_; }

    //! Returns the index of the hole in the face that this edge is a part
    //! of, or -1 for edges forming the outer boundary.
    int GetFaceHoleIndex() const { return face_hole_index_; }

    //! Returns the index of the edge in whichever list of edges it is in
    //! inside its face (outer boundary or hole).
    int GetIndexInFace() const { return index_in_face_; }

    //! Returns the corresponding edge in the opposite direction on the face on
    //! the other side of this edge. The opposite edge goes from v1 to v0. This
    //! is null by default.
    Edge * GetOppositeEdge() const { return opposite_edge_; }

    //! Returns a unit vector from v0 to v1.
    Vector3f GetUnitVector() const;

  private:
    Vertex  &v0_;                       //!< Vertex at the start of the edge.
    Vertex  &v1_;                       //!< Vertex at the end of the edge.
    Face    &face_;                     //!< The face the edge is part of.
    int     face_hole_index_;           //!< Index within hole; -1 if outer.
    int     index_in_face_;             //!< Index within Face's list of edges.
    Edge    *opposite_edge_ = nullptr;  //! Opposite edge.

    //! Connects the Edge to an opposite edge.
    void ConnectOpposite_(Edge &opposite);

    friend class PolyMesh;
};

// ----------------------------------------------------------------------------
// PolyMesh::Face class definition.
// ----------------------------------------------------------------------------

//! A face containing 3 or more vertices/edges.
class PolyMesh::Face : public PolyMesh::Feature {
  public:
    //! Constructs a Face with the given ID.
    Face(int id) : Feature("F", id) {}

    //! Returns the normal to the face.
    const Vector3f & GetNormal() const;

    //! Returns the number of holes in the face, which is typically 0.
    size_t GetHoleCount() const { return hole_edges_.size(); }

    //! Returns an ordered vector of directed edges defining the outer border
    //! of the face, traveling counterclockwise when viewed from outside the
    //! face.
    const EdgeVec & GetOuterEdges() const { return outer_edges_; }

    //! Returns a vector of vectors of directed edges forming holes in the
    //! face, traveling clockwise. This is typically empty.
    const std::vector<EdgeVec> & GetHoleEdges() const { return hole_edges_; }

  private:
    //! Ordered vector of directed edges defining the outer border of the
    //! face, traveling counterclockwise when viewed from outside the face.
    EdgeVec outer_edges_;

    //! Vector of vectors of directed edges forming holes in the face,
    //! traveling clockwise. This is typically empty.
    std::vector<EdgeVec> hole_edges_;

    //! Adds an edge.
    void AddEdge_(int hole_index, Edge &edge);

    //! Normal to the face, computed only when necessary.
    mutable Vector3f normal_;

    friend class PolyMesh;
};
