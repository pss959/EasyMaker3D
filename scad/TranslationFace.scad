//=============================================================================
// One of these is used for each face of the TranslationManip in MakerVR.
//=============================================================================

module Pyramid(baseSize = 10, height = 10) {
     bsh = baseSize / 2;
     points = [
          // Base.
          [ bsh,  bsh, 0],
          [ bsh, -bsh, 0],
          [-bsh, -bsh, 0],
          [-bsh,  bsh, 0],
          // Pointy top.
          [0, 0, height]
     ];
     faces = [
          // Sides.
          [0, 4, 1], [1, 4, 2], [2, 4, 3], [3, 4, 0],
          // Base
          [1, 3, 0], [2, 3, 1]
     ];

     polyhedron(points = points, faces = faces);
}

module TranslationFace() {
     translate([0, 0, 1]) cube([10, 10, 2], center = true);
     translate([0, 0, 2]) Pyramid(baseSize = 10, height = 2);
}

TranslationFace();
