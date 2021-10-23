module ToggleEdgeTargetIcon() {
  kL = 6;
  cube([2, 2, 2], center = true);
  translate([0, 0, kL / 2]) cylinder(r  = .4, h = kL, center = true);
  translate([0, 0,  kL]) cylinder(r1 =  1, r2 = 0, h = 2.4, center = true);
}

rotate([-45, 90, 0]) ToggleEdgeTargetIcon($fn = 60);
