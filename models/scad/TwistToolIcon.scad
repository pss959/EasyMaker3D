use <Pencil.scad>
use <ConvertTwistIcon.scad>

module TwistToolIcon() {
  scale(7 * [1, 1, 1]) ConvertTwistIcon();
  translate([2, -10, 12]) rotate([90, 0, 0]) Pencil($fn = 64);
}

TwistToolIcon();
