module RoundedRect(width, height, depth, cornerRadius) {
  minkowski() {
    cube([width, height, depth], center = true);
    cylinder(r = cornerRadius, h = depth);
  }
}

module OpenSessionPanelIcon() {
  module Rect() {
    RoundedRect(width = 11, height = 16, depth = 1, cornerRadius = 2);
  }
  module Slicer() {
    translate([8, 10, 0]) rotate([0, 0, -45])
      cube([20, 10, 12], center = true);
  }
  module MainPart() {
    difference() {
      Rect();
      Slicer();
    }
  }
  module Corner() {
    translate([10, 10, 2.4]) mirror([1, 1, 0]) intersection() {
      Rect();
      Slicer();
    }
  }
  MainPart();
  Corner();
}

OpenSessionPanelIcon($fn = 40);
