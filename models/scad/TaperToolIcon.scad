use <Pencil.scad>
use <ConvertTaperIcon.scad>

module TaperToolIcon() {
  scale([3, 3, 1]) ConvertTaperIcon();
  translate([7, 9, 10]) Pencil();
}

$fn = 64;
TaperToolIcon();
