//=============================================================================
// Pencil added to an icon to show it is used for editing.
// =============================================================================

include <Pencil.scad>

module CylinderToolIcon() {
  rotate([90, 0, 0]) cylinder(r = 8, h = 16, center = true);
  translate([0, 0, 10]) Pencil();
}

$fn = 64;
CylinderToolIcon();
