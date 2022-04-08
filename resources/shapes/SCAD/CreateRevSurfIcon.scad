//=============================================================================
// Icon used for creating a surface of revolution model.
//=============================================================================

module CreateRevSurfIcon() {
  points=[[0,0],
          [2,0],
          [1.7,.5],
          [1,1],
          [1.3,2.2],
          [1,2.5],
          [2,3],
          [2,4],
          [0,5]];

  rotate_extrude() polygon(points);
}

CreateRevSurfIcon($fn=72);
