//=============================================================================
// 3D icon used for ImportModel in MakerVR.
// =============================================================================

module CreateImportedModelIcon() {
     w = 10;
     h =  8;
     t =  2;
     d =  4;
     module _Bottom() {
          translate([0, -h/2, 0]) cube([w, t, d], center = true);
          translate([-w/2, 0, 0]) cube([t, h, d], center = true);
          translate([ w/2, 0, 0]) cube([t, h, d], center = true);
          translate([-w/2, -h/2, 0]) cylinder(d=t, h=d, center = true);
          translate([ w/2, -h/2, 0]) cylinder(d=t, h=d, center = true);
     }
     module _Arrow() {
          rotate([90, 0, 0]) cylinder(r=.7, h=4, center = true);
          translate([0, -3, 0]) rotate([90, 0, 0])
               cylinder(d1=d, d2=0, h=2.5, center = true);
     }
     _Bottom();
     translate([0, 1.8, 0]) _Arrow();
}

CreateImportedModelIcon($fn = 120);
