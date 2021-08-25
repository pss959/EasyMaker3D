uniform mat4 uBiasMatrix;
uniform vec3 uLightPos;

attribute vec3 aVertex;

varying vec3 vToLight;

void main() {
  vToLight    = uLightPos - aVertex;
  gl_Position = uBiasMatrix * vec4(aVertex, 1.);
}
