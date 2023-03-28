//=============================================================================
// Icon used for creating an extruded model.
//=============================================================================

module CreateExtrudedIcon() {
  points=[
          [-1, -1],
          [ 0, -.4],
          [ 1, -1],
          [ .6, 0],
          [ 1,  1],
          [ 0, .4],
          [-1,  1],
          [-.6, 0],
          ];

  rotate(70, [1, 0, 0]) linear_extrude(height=2, center=true, $fn=1)
    scale([2,2,2]) polygon(points);
}

CreateExtrudedIcon();
