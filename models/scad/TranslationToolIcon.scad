use <TranslationFace.scad>

module TranslationToolIcon() {
  module Axis_() {
    kL = 8;
    kS = .5;
    cube([2 * kL, 1, 1], center = true);
    translate([-kL, 0, 0]) rotate([0, -90, 0]) scale(kS) TranslationFace();
    translate([ kL, 0, 0]) rotate([0,  90, 0]) scale(kS) TranslationFace();
  }

  Axis_();
  rotate([0, 0, 90]) Axis_();
  rotate([0, 90, 0]) Axis_();
}

TranslationToolIcon($fn = 72);
