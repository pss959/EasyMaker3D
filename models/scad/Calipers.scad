//=============================================================================
// 3D icon used for ImportModel.
// =============================================================================

module Calipers() {
     // Shaft sizes.
     sw = 2;
     sh = 20;
     sd = 2;

     // Jaw sizes.
     jw = 12;
     jh = 2;
     jd = 3;

     module Shaft() {
          cube([sw, sh, sd], center = true);
     }

     module EndJaw() {
          points = [
               [0, 0],
               [jw, 0],
               [jw, jh/2],
               [0, jh],
               ];
          linear_extrude(height = jd) polygon(points);
     }

     module MovingJaw() {
          points = [
               [0, 0],
               [jw, 0],
               [jw, -jh/2],
               [0, -jh],
               ];
          linear_extrude(height = jd) polygon(points);
     }

     module Ring() {
          difference() {
               cylinder(r = 3, h = 2, center = true);
               cylinder(r = 1.5, h = 3, center = true);
          }
     }

     Shaft();
     translate([-2, 10, -jd/2]) EndJaw();
     translate([-2, 4,  -jd/2]) MovingJaw();
     translate([8,  7,      0]) Ring();
}

Calipers($fn = 120);
