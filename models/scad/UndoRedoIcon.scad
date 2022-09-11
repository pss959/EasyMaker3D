//=============================================================================
// 3D icon used for Undo and Redo. (Rotate or mirror to get the other.)
// =============================================================================

use </home/pss/git/projects/maker/scad/Torus.scad>

module UndoRedo() {
  translate([0, -6, 0]) union() {
    intersection() {
      Torus(inner_radius = 2, outer_radius = 10);
      translate([-1, 15, 0]) rotate([0, 0, 30])
        cube([22, 23, 20], center = true);
    }
    translate([9.5, 4.4, 0]) rotate([90, 0, 40])
      cylinder(r1 = 4, r2 = 0, h = 8, center = true);
  }
}

$fn = 120;
UndoRedo();
