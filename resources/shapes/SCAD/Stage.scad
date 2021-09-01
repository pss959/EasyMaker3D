// A rounded cylinder with reasonable resolution to use as the stage.
// Axis is aligned with the Y axis. All dimensions are -1 to +1.

module Stage() {
  kRadius         = 32;
  kHeight         = 1;
  kRoundingRadius = .4;
  minkowski() {
    cylinder(r = kRadius - kRoundingRadius,
             h = kHeight - 2 * kRoundingRadius,
             center = true);
    sphere(r = kRoundingRadius);
  }
}

// Rotate so that Y is up.
rotate([90, 0, 0]) Stage($fn = 120);
