use <Pencil.scad>
use <CreateRevSurfIcon.scad>

module RevSurfToolIcon() {
  rotate([-90, 0, 0]) scale([4, 4, 4]) CreateRevSurfIcon();
  translate([1, 10, 10]) Pencil();
}

$fn = 64;
RevSurfToolIcon();
