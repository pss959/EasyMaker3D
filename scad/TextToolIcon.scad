use <Pencil.scad>

module TextToolIcon() {
  linear_extrude(height = 6) {
    text(text = "T", font="Liberation Serif:style=Bold",
         size = 20, halign = "center", valign = "center");
  }
  translate([1, 1, 10]) Pencil();
}

TextToolIcon();
