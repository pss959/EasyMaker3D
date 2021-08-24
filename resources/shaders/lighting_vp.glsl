uniform mat4 uProjectionMatrix;
uniform mat4 uModelviewMatrix;
uniform mat4 uBiasMatrix;

attribute vec3 aVertex;
attribute vec3 aNormal;
attribute vec2 aTexCoords;

varying vec3 vPosition;
varying vec3 vNormal;
varying vec2 vTexCoords;
varying vec4 vShadowPos;

void main(void) {
  vec4 vertex = vec4(aVertex, 1.);

  vPosition  = aVertex;
  vNormal    = aNormal;
  vTexCoords = aTexCoords;
  vShadowPos = uBiasMatrix * vertex;

  gl_Position = uProjectionMatrix * uModelviewMatrix * vertex;
}
