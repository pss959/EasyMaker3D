use <Pencil.scad>
use <ConvertMirrorIcon.scad>

module MirrorToolIcon() {
  scale([1.2, 1, 1.2]) ConvertMirrorIcon();
  translate([0, -14, 0]) rotate([90, 0, 0]) Pencil();
}

$fn = 64;
MirrorToolIcon();
