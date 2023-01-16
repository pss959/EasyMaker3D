//=============================================================================
// 3D icon used for MirrorTool. The tool itself is just 3 planes, which isn't
// useful as an icon.
// =============================================================================

use </home/pss/git/projects/maker/scad/Torus.scad>

module ConvertMirrorIcon() {
     module _Arrow() {
          module _ArrowHead() {
               cylinder(r1 = 2.5, r2 = 0, h = 6, center = true);
          }
          translate([0, -6, 0]) union() {
               intersection() {
                    Torus(inner_radius = 1.2, outer_radius = 10);
                    translate([0, 12, 0]) rotate([0, 0, 45])
                         cube([22, 23, 20], center = true);
               }
               translate([ 9.5, 4.4, 0]) rotate([90, 0,  40]) _ArrowHead();
               translate([-9.5, 4.4, 0]) rotate([90, 0, -40]) _ArrowHead();
          }
     }
     module _Wall() {
          cube([1, 15, 15], center = true);
     }
     _Wall();
     translate([0, -6, 0]) rotate([200, 0, 0]) _Arrow();
}

$fn = 120;
ConvertMirrorIcon();
