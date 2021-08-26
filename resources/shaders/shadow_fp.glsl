#version 330 core

//uniform vec2 uDepthRange;  // Min/max depth values.

//out float depth;

void main() {
#if XXXX
  float min_depth = uDepthRange.x;
  float max_depth = uDepthRange.y;
  float frag_depth = gl_FragCoord.z / gl_FragCoord.w;
  depth = clamp((frag_depth - min_depth) / (max_depth - min_depth), 0, 1);
  depth = frag_depth;
#endif
}
