#version 330 core

uniform vec2 uDepthRange;

in vec3 vToLight;

void main() {
  float dist = length(vToLight);
  float min_depth = uDepthRange.x;
  float max_depth = uDepthRange.y;
  float depth = (dist - min_depth) / (max_depth - min_depth);
  float depth_sq = depth * depth;

  /* XXXX
  float dx = dFdx(depth);
  float dy = dFdy(depth);
  depth_sq += 0.25 * (dx * dx + dy * dy);
  */

  // store depths and tex coords
  gl_FragColor.rg = vec2(depth, depth_sq);
}
