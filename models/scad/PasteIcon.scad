//=============================================================================
// Icon used for the Paste operation: a stylized clipboard.
//=============================================================================

module RoundedRect(width, height, depth, roundRadius) {
  minkowski() {
    cube([width, height, depth], center = true);
    sphere(r = roundRadius);
  }
}

module Board() {
  RoundedRect(width = 10, height = 12, depth = .2, roundRadius = .6);
}

module Clip() {
  union() {
    cube([6, 5, 2], center = true);
    translate([0, 3, 0]) difference() {
      cylinder(r =  1.2,  h = 1, center = true);
      cylinder(r = .5 , h = 2, center = true);
    }
  }
}

module PasteIcon() {
  union() {
    Board();
    translate([0, 6, 0]) Clip();
  }
}

$fn = 40;
PasteIcon();
