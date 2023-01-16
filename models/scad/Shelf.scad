//=============================================================================
// Shelf used for placing icons on.
//=============================================================================

module Shelf() {
     minkowski() {
          kDiameter = .4;
          cube([20 - kDiameter, 1 - kDiameter, 10 - kDiameter], center = true);
          sphere(d = kDiameter, $fn = 30);
     }
}

Shelf();
