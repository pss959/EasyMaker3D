//=============================================================================
// 3D icon used for TwistTool. 
// =============================================================================

module ConvertTwistIcon() {
  linear_extrude(height = 2.5, center = 2, twist = -90, slices = 8)
    circle(r = 1, $fn = 5);
}

ConvertTwistIcon();
