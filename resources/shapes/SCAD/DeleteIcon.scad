//=============================================================================
// Icon used for the Delete operation: a big 'X'.
//=============================================================================

module Diagonal() {
  intersection() {
    cube([10, 10, 2], center = true);
    rotate([0, 0, 40]) cube([2, 20, 4], center = true);
  }
}

module X() {
  union() {
    Diagonal();
    mirror([0, 1, 0]) Diagonal();
  }
}

module DeleteIcon() {
  X();
}

$fn = 30;
DeleteIcon();
