//=============================================================================
// Icon used for the Cut operation. Scissors, of course.
//=============================================================================

module Handle() {
  scale([1, .8, 1]) {
    difference() {
      cylinder(r =  2,  h = 1, center = true);
      cylinder(r = .8, h = 2, center = true);
    }
  }
}


module Blade() {
  translate([-2.4, -5, 0]) scale([1, 1, 1]) {
    intersection() {
      cylinder(r = 8, h = 1, center = true);
      translate([0, 4.8, 0]) cube([20, 1.8, 4], center = true);
    }
  }
}

module Side() {
  rotate([0, 0, 28]) translate([-5.4, 0, 0]) {
    Handle();
    translate([9.6, 0, 0]) Blade();
  }
}

module Scissors() {
  union() {
    Side();
    mirror([0, 1, 0]) Side();
  }
}

module CutIcon() {
  Scissors();
}

$fn = 30;
CutIcon();
