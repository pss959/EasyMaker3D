module ConvertBevelIcon () {
     w = 3;
     points = [
          [0, 0],
          [w, 0],
          [w+1, 1],
          [w+2, 2.5],
          [w+2, 3.5],
          [w+1, 5.5],
          [w, 6],
          [0, 6],
     ];
     linear_extrude(height = 5) polygon(points);
}

ConvertBevelIcon();
