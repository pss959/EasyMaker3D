module OpenSessionPanelIcon() {
  ht  = 4;
  rad = 6;
  module Tooth_() {
    module Differ_(plus_minus) {
      translate([plus_minus * 2.2, 0, 0])
        rotate([0, 0, plus_minus * 10])
        cube([2, 4, 2 * ht], center = true);
    }
    difference() {
      cube([3, 2, ht], center = true);
      Differ_(1);
      Differ_(-1);
    }
  }

  difference() {
    cylinder(h = ht,     r = rad,      center = true);
    cylinder(h = 2 * ht, r = .6 * rad, center = true);
  }
  for (i = [0:7]) {
    rotate([0, 0, 45 * i]) translate([0, 6.8, 0]) Tooth_();
  }
}

OpenSessionPanelIcon($fn = 60);
