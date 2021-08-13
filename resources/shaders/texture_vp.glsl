uniform mat4 uProjectionMatrix;
uniform mat4 uModelviewMatrix;

attribute vec3 aVertex;
attribute vec2 aTexCoords;

varying vec2 vTexCoords;

void main(void) {
  vTexCoords = aTexCoords;
  gl_Position = uProjectionMatrix * uModelviewMatrix * vec4(aVertex, 1);
}
