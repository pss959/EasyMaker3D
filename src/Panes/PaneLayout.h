#pragma once

#include "Enums/PaneOrientation.h"
#include "Math/Types.h"

class ContainerPane;

/// The PaneLayout class implements functions to help with Pane layout in
/// derived ContainerPane classes. Each layout function is supplied
/// ContainerPane whose sub-Panes are to be laid out.
///
/// \ingroup Pane
class PaneLayout {
  public:
    enum Direction { kHorizontal, kVertical };

    /// Computes and returns the base size for all sub-Panes in a horizontal or
    /// vertical direction using the given padding and spacing values.
    static Vector2f ComputeLinearSize(const ContainerPane &container,
                                      PaneOrientation dir,
                                      float padding, float spacing);

    /// Lays out sub-Panes in the given direction into a rectangle of the given
    /// size, using the given padding and spacing values. If \p offset_forward
    /// is true, all sub-Panes will be translated a small amount in Z to put it
    /// in front.
    static void LayOutLinearly(const ContainerPane &container,
                               PaneOrientation dir, const Vector2f &size,
                               float padding, float spacing,
                               bool offset_forward = false);

  private:
    class Impl_;
};
