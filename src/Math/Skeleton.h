#pragma once

#include <vector>

#include "Math/Types.h"

/// Templated base class for straight skeleton. XXXX What and why.
///
/// \ingroup Math
template <typename PointType> class Skeleton {
  public:
    /// One Vertex of the skeleton.
    struct Vertex {
        /// Index of the vertex in the source structure, or -1 if the vertex
        /// was created for the skeleton.
        int       source_index;
        /// Vertex location.
        PointType point;
        /// Distance of the vertex to the contour points it bisects. This will
        /// be 0 for the vertices corresponding to source vertices.
        float     distance;
    };

    /// One edge of the skeleton. Edges are stored in one direction only, with
    /// v0_index < v1_index.
    struct Edge {
        size_t v0_index;
        size_t v1_index;
    };

    virtual ~Skeleton() {}

    /// Returns the vertices of the skeleton.
    const std::vector<Vertex> & GetVertices() const { return vertices_; }

    /// Returns the bisector edges of the skeleton.
    const std::vector<Edge>   & GetEdges()    const { return edges_; }

  protected:
    /// Constructor is protected to make this abstract.
    Skeleton() {};

    /// Sets the vertices.
    void SetVertices(const std::vector<Vertex> &vertices) {
        vertices_ = vertices;
    }

    /// Sets the edges.
    void SetEdges(const std::vector<Edge> &edges) {
        edges_ = edges;
    }

  private:
    std::vector<Vertex> vertices_;
    std::vector<Edge>   edges_;
};
