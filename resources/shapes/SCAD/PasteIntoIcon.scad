//=============================================================================
// Icon used for the Paste-Into operation: the Paste icon inside a circle.
//=============================================================================

use <PasteIcon.scad>

module Circle() {
  difference() {
    cylinder(r = 13, h = 2, center = true);
    cylinder(r = 11, h = 4, center = true);
  }
}

module PasteIntoIcon() {
  translate([0, -1, 0]) PasteIcon();
  Circle();
}

$fn = 40;
PasteIntoIcon();
