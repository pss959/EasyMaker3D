module CombineHullIcon () {
  module Sphere_() { sphere(r = 1); }
  d = 2;
  h = 3;
  hull() {
    translate([-d, -d, 0]) Sphere_();
    translate([-d,  d, 0]) Sphere_();
    translate([ d, -d, 0]) Sphere_();
    translate([ d,  d, 0]) Sphere_();
    translate([ 0,  0, h]) Sphere_();
  }
}

CombineHullIcon($fn = 40);
