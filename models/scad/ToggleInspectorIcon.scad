//=============================================================================
// Icon used to toggle the Inspector: An object inside a magnifying glass.
//=============================================================================

module MagnifyingGlass() {
  module Glass() {
    difference() {
      cylinder(r = 14, h = 4, center = true);
      cylinder(r = 11, h = 8, center = true);
    }
  }
  module Handle() {
    cylinder(r = 2, h = 14);
  }
  Glass();
  translate([-9, -9, 0]) rotate([90, 0, -45]) Handle();
}

module Object() {
  cylinder(r = 7, h = 12, center = true);
}

module InspectorIcon() {
  rotate([-5, -10, 0]) MagnifyingGlass();
  rotate([80, 0, -10]) Object();
}

$fn = 80;
InspectorIcon();
