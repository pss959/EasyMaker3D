uniform mat4 uProjectionMatrix;
uniform mat4 uModelviewMatrix;

attribute vec3 aVertex;
attribute vec3 aNormal;
attribute vec2 aTexCoords;

varying vec3 vNormal;
varying vec2 vTexCoords;

void main(void) {
  vNormal     = aNormal;
  vTexCoords  = aTexCoords;
  gl_Position = uProjectionMatrix * uModelviewMatrix * vec4(aVertex, 1);
}
