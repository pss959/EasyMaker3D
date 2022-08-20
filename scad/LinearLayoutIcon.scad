module LinearLayoutIcon() {
  kL = 12;
  module Cube_() { cube([2, 2, 2], center = true); }

  cylinder(r  = .4, h = kL, center = true);
  translate([0, 0,  0])      Cube_();
  translate([0, 0,  kL / 2]) Cube_();
  translate([0, 0, -kL / 2]) Cube_();
}

rotate([-45, 90, 0]) LinearLayoutIcon($fn = 60);
