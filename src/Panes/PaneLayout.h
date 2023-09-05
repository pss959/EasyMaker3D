#pragma once

#include "Math/Types.h"
#include "Panes/Pane.h"

/// The PaneLayout class implements functions to help with Pane layout in
/// derived ContainerPane classes. Each layout function is supplied a vector of
/// Pane instances to lay out.
///
/// \ingroup Pane
class PaneLayout {
  public:
    /// Computes and returns the base size for all Panes in a horizontal
    /// direction using the given padding and spacing values.
    static Vector2f ComputeHorizontalSize(const Pane::PaneVec &panes,
                                          float padding, float spacing);

    /// Computes and returns the base size for all Panes in a vertical
    /// direction using the given padding and spacing values.
    static Vector2f ComputeVerticalSize(const Pane::PaneVec &panes,
                                        float padding, float spacing);

    /// Lays out Panes horizontally in a rectangle of the given size, using the
    /// given padding and spacing values. If \p offset_forward is true, all
    /// Panes will be translated a small amount in Z to put it in front.
    static void LayOutHorizontally(const Pane::PaneVec &panes,
                                   const Vector2f &size,
                                   float padding, float spacing,
                                   bool offset_forward = false);

    /// Lays out Panes vertically in a rectangle of the given size, using the
    /// given padding and spacing values.
    static void LayOutVertically(const Pane::PaneVec &panes,
                                 const Vector2f &size,
                                 float padding, float spacing,
                                 bool offset_forward = false);

  private:
    class Impl_;
};
