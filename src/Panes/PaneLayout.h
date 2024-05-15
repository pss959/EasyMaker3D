#pragma once

#include <vector>

#include "Enums/PaneOrientation.h"
#include "Math/Types.h"

class ContainerPane;

/// The PaneLayout class implements functions to help with Pane layout in
/// derived ContainerPane classes. Each layout function is supplied
/// ContainerPane whose sub-Panes are to be laid out.
///
/// \ingroup Panes
class PaneLayout {
  public:
    /// This struct is used for computing grid sizes. It represents cells in
    /// one direction for the grid.
    struct GridData {
        size_t            count;    ///< Number of cells in this direction.
        float             spacing;  ///< Spacing between cells.
        std::vector<bool> expands;  ///< Whether each cell expands.
    };

    /// Computes and returns the base size for all sub-Panes in a horizontal or
    /// vertical direction using the given padding and spacing values.
    static Vector2f ComputeLinearSize(const ContainerPane &container,
                                      PaneOrientation dir,
                                      float padding, float spacing);

    /// Computes and returns the base size for a grid whose cell data is
    /// specified as two GridData instances. This assumes the sub-Panes are
    /// laid out in row-major order.
    static Vector2f ComputeGridSize(const ContainerPane &container,
                                    float padding,
                                    const GridData &horiz_data,
                                    const GridData &vert_data);

    /// Lays out sub-Panes in the given direction into a rectangle of the given
    /// size, using the given padding and spacing values. If \p offset_forward
    /// is true, all sub-Panes will be translated a small amount in Z to put it
    /// in front.
    static void LayOutLinearly(const ContainerPane &container,
                               PaneOrientation dir, const Vector2f &size,
                               float padding, float spacing,
                               bool offset_forward = false);

    /// Lays out sub-Panes in a grid into a rectangle of the given size with
    /// the given padding.
    static void LayOutGrid(const ContainerPane &container,
                           const Vector2f &size, float padding,
                           const GridData &col_data, const GridData &row_data);

  private:
    class Impl_;
};
