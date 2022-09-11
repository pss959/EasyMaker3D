module ToggleAxisAlignedIcon() {
  module Axis_() {
    kL = 6;
    cube([2 * kL, 1, 1], center = true);
  }
  Axis_();
  rotate([0, 0, 90]) Axis_();
  rotate([0, 90, 0]) Axis_();
}

ToggleAxisAlignedIcon();
