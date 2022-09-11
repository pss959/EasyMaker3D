// Ring used in both the ColorTool geometry and the ColorToolIcon.

module ColorToolRing() {
  difference() {
    cylinder(r = 1,   h = .11, center = true);
    cylinder(r = .45, h = .2, center = true);
  }
}

ColorToolRing($fn = 60);
