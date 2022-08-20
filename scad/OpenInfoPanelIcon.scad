module OpenInfoPanelIcon() {
  difference() {
    cylinder(r = 13, h = 2, center = true);
    cylinder(r = 10, h = 4, center = true);
  }
  linear_extrude(2)
    text("i", halign = "center", valign = "center", size = 16,
         font="Liberation Sans:style=Bold");
}

OpenInfoPanelIcon($fn = 40);
