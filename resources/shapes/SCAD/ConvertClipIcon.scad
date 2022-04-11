module ConvertClipIcon() {
     cylinder(r = 5, h = 12, center = true);
     rotate([20, 0, 0]) cube([18, 18, 1], center = true);
}

ConvertClipIcon($fn = 40);
