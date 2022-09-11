use <Pencil.scad>
use <ConvertClipIcon.scad>

module ClipToolIcon() {
  scale(1.6 * [1, 1, 1]) ConvertClipIcon();
  translate([0, -14, 0]) rotate([90, 0, 0]) Pencil();
}

$fn = 64;
ClipToolIcon();
