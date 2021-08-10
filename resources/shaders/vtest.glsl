uniform mat4 uProjectionMatrix;
uniform mat4 uModelviewMatrix;

attribute vec3 aVertex;

void main(void) {
  vec4 position = uProjectionMatrix * uModelviewMatrix * vec4(aVertex, 1);
  gl_Position = position;
}
