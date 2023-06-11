use <CombineCSGIcon.scad>
use <Pencil.scad>

module CSGToolIcon() {
  scale(7 * [1, 1, 1]) DifferenceIcon($fn = 72);
  translate([1, 0, 10]) Pencil($fn = 64);
}

CSGToolIcon();
