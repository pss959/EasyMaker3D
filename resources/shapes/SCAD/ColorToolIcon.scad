module ColorToolIcon() {
  difference() {
    cylinder(r = 10,  h = 1, center = true);
    cylinder(r = 4.5, h = 2, center = true);
  }
  cylinder(r = 2.5, h = 1, center = true);
}

// Rotate so that Y is up.
rotate([90, 0, 0]) ColorToolIcon($fn = 60);
