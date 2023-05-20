//=============================================================================
// 3D icon used for BendTool. 
// =============================================================================

module ConvertBendIcon() {
  linear_extrude(height = 2.5, center = 2, bend = -90, slices = 8)
    circle(r = 1, $fn = 5);
}

ConvertBendIcon();
