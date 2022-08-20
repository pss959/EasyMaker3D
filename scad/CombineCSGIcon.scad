module Object1() {
     color([.9, .7, .8]) cube([2, 2, 2], center = true);
}
module Object2() {
     color([.8, .8, .9]) translate([1, 1, 1]) sphere(r = 1.3);
}
module Ghost() {
     color([1, 1, 1, .1]) children();
}

module UnionIcon() {
     union() {
          Object1();
          Object2();
     }
}

module IntersectionIcon() {
     intersection() {
          Object1();
          Object2();
     }
     //Ghost() { Object1(); }
     //Ghost() { Object2(); }
}

module DifferenceIcon() {
     difference() {
          Object1();
          Object2();
     }
     //Ghost() { Object2(); }
}

$fn = 72;
//UnionIcon();
IntersectionIcon();
//DifferenceIcon();
