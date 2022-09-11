use <Pencil.scad>
use <CreateImportedModelIcon.scad>

module ImportToolIcon() {
  scale([2, 2, 1]) CreateImportedModelIcon();
  translate([1, 2, 6]) Pencil();
}

$fn = 64;
ImportToolIcon();
