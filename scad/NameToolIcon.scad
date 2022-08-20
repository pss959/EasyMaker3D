use <Pencil.scad>

module NameToolIcon() {
  linear_extrude(height = 6) {
    text(text = "N", font="Liberation Serif:style=Bold",
         size = 20, halign = "center", valign = "center");
  }
  translate([1, 1, 10]) Pencil();
}

NameToolIcon();
