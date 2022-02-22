use <Pencil.scad>
use <ConvertBevelIcon.scad>

module BevelToolIcon() {
  scale([3, 3, 1]) ConvertBevelIcon();
  translate([7, 9, 10]) Pencil();
}

$fn = 64;
BevelToolIcon();
