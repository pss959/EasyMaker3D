use <Pencil.scad>

// Torus parallel to the XY plane.
module Torus(inner_radius = 1, outer_radius = 5) {
  rotate_extrude() {
    translate([outer_radius, 0, 0]) circle(r = inner_radius);
  }
}

module TorusToolIcon() {
  rotate([-10, 0, 0]) Torus(outer_radius = 7, inner_radius = 2);
  translate([0, 0, 5]) Pencil();
}

$fn = 64;
TorusToolIcon();
