module ScaleToolIcon() {
  module Axis_() {
    kL = 8;
    kC = 2.2;
    cube([2 * kL, 1, 1], center = true);
    translate([-kL, 0, 0]) cube([kC, kC, kC], center = true);
    translate([ kL, 0, 0]) cube([kC, kC, kC], center = true);
  }

  Axis_();
  rotate([0, 0, 90]) Axis_();
  rotate([0, 90, 0]) Axis_();
}

ScaleToolIcon($fn = 72);
