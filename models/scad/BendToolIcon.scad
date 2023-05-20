use <Pencil.scad>
use <ConvertBendIcon.scad>

module BendToolIcon() {
  scale(7 * [1, 1, 1]) ConvertBendIcon();
  translate([2, -10, 12]) rotate([90, 0, 0]) Pencil($fn = 64);
}

BendToolIcon();
