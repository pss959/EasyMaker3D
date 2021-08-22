uniform mat4 uProjectionMatrix;
uniform mat4 uModelviewMatrix;

attribute vec3 aVertex;
attribute vec3 aNormal;

varying vec3 vNormal;

void main(void) {
  vNormal     = aNormal;
  gl_Position = uProjectionMatrix * uModelviewMatrix * vec4(aVertex, 1);
}
