//=============================================================================
// Icon used for toggling whether edges are shown. This is a pyramid of thin
// cubes that sort of look like edges.
//=============================================================================

kThick  = 1;
kLength = 10;
kHalf   = .5 * kLength;

module XEdge() {
  rotate([0, 90, 0]) cylinder(h = kLength, d = kThick, center = true);
}
module YEdge() {
  rotate([0, 0, 90]) XEdge();
}
module Corner() {
  sphere(d = kThick);
}

module Base() {
  translate([0, -kHalf, 0]) XEdge();
  translate([0,  kHalf, 0]) XEdge();
  translate([-kHalf, 0, 0]) YEdge();
  translate([ kHalf, 0, 0]) YEdge();
  translate([-kHalf, -kHalf, 0]) Corner();
  translate([-kHalf,  kHalf, 0]) Corner();
  translate([ kHalf, -kHalf, 0]) Corner();
  translate([ kHalf,  kHalf, 0]) Corner();
}

module Top() {
  module Slant() {
    translate([.5 * kHalf, .5 * kHalf, .707 * kHalf])
      rotate([45, 45, 45]) XEdge();
  }
  rotate([0, 0,   0]) Slant();
  rotate([0, 0,  90]) Slant();
  rotate([0, 0, 180]) Slant();
  rotate([0, 0, 270]) Slant();
  translate([0, 0, 1.414 * kHalf]) Corner();
}

module ShowEdgesIcon() {
  Base();
  Top();
}

$fn = 32;
ShowEdgesIcon();
