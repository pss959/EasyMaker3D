module RadialLayoutIcon() {
  kR = 10;
  module Cube_() { cube([3, 3, 3], center = true); }

  difference() {
    cylinder(r = kR,     h = 1, center = true);
    cylinder(r = kR - 1, h = 2, center = true);
    translate([0, -.75 * kR, 0]) cube([2 * kR, .5 * kR, 4], center = true);
  }

  translate([0, kR - .5, 0])          Cube_();
  translate([-.88 * kR, -.4 * kR, 0]) Cube_();
  translate([ .88 * kR, -.4 * kR, 0]) Cube_();
}

RadialLayoutIcon($fn = 60);
