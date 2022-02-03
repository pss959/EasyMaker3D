//=============================================================================
// Icon used for the Copy operation.
//=============================================================================

module RoundedRect(width, height, depth, roundRadius) {
  minkowski() {
    cube([width, height, depth], center = true);
    sphere(r = roundRadius);
  }
}

module CopyIcon() {
  module RR() {
    RoundedRect(width = 10, height = 12, depth = .2, roundRadius = 1.2);
  }
  RR();
  translate([-3, -4, 4]) RR();
}

$fn = 40;
CopyIcon();
