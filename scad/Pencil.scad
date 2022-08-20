//=============================================================================
// Pencil added to an icon to show it is used for editing.
// =============================================================================

module Pencil() {
  rotate([90, 0, -45]) union() {
    radius    = 1.8;
    cylHeight = 9;
    coneHeight = 4;
    cylinder(r = radius, h = cylHeight, center = true);
    translate([0, 0, (cylHeight + coneHeight) / 2])
      cylinder(r1 = radius, r2 = 0, h = coneHeight, center = true);
  }
}

$fn = 40;
Pencil();
