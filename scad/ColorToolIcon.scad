use <ColorToolRing.scad>

module ColorToolIcon() {
  scale([10, 10, 10]) ColorToolRing();
  cylinder(r = 2.5, h = 1, center = true);
}

// Rotate so that Y is up.
rotate([90, 0, 0]) ColorToolIcon($fn = 60);
