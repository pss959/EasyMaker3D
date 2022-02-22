use <Pencil.scad>

module CylinderToolIcon() {
  rotate([90, 0, 0]) cylinder(r = 8, h = 16, center = true);
  translate([0, 0, 10]) Pencil();
}

$fn = 64;
CylinderToolIcon();
