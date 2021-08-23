varying vec3 vToLight;

void main() {
  float depth = gl_FragCoord.z;
  gl_FragColor.rg = vec2(depth, depth * depth);
}
