module ConvertTaperIcon () {
     points = [
          [0, 0],
          [5, 0],
          [3, 6],
          [2, 6],
     ];
     linear_extrude(height = 4) polygon(points);
}

ConvertTaperIcon();
