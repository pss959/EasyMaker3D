//=============================================================================
// 3D icon used for BendTool. 
// =============================================================================

module ConvertBendIcon() {
  bend = 110;
  translate([.6, 0, 0])
    rotate ([90, bend - 180, 0])
    rotate_extrude(angle = bend, convexity = 10, $fn = 32)
    translate([2, 0, 0]) circle(r = .6, $fn = 18);
}

ConvertBendIcon();
