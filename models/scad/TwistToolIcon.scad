use <Pencil.scad>
use <ConvertTwistIcon.scad>

module TwistToolIcon() {
  scale([1.2, 1, 1.2]) ConvertTwistIcon();
  translate([0, -14, 0]) rotate([90, 0, 0]) Pencil();
}

$fn = 64;
TwistToolIcon();
