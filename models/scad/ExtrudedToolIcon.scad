use <Pencil.scad>
use <CreateExtrudedIcon.scad>

module ExtrudedToolIcon() {
  rotate([-90, 0, 0]) scale([4, 4, 4]) CreateExtrudedIcon();
  translate([1, 7, 6]) Pencil();
}

$fn = 64;
ExtrudedToolIcon();
