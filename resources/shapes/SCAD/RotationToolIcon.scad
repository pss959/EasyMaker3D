module Torus(inner_radius = 1, outer_radius = 5) {
  rotate_extrude() {
    translate([outer_radius, 0, 0]) circle(r = inner_radius);
  }
}

module RotationToolIcon() {
  module Torus_() { Torus(.8, 10); }
  Torus_();
  rotate([90, 0, 0]) Torus_();
  rotate([0, 90, 0]) Torus_();
}

RotationToolIcon($fn = 72);
