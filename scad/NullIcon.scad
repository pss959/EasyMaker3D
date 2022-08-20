//=============================================================================
// Icon used when there is no operation available: a null sign.
//=============================================================================

module NullIcon() {
  difference() {
    cylinder(r = 12, h = 4, center = true);
    cylinder(r =  8, h = 6, center = true);
  }
  rotate([0, 0, -45]) cube([4, 34, 4], center = true);
}

$fn = 64;
NullIcon();
