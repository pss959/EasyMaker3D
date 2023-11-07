#version 330 core

uniform mat4  uProjectionMatrix;
uniform mat4  uViewMatrix;
uniform mat4  uModelMatrix;
uniform float uZOffset;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoords;

// Hack to shut up warnings about unused attributes.
float Unused(vec3 a, vec2 b) {
  return (100 * a[0]) < -1000 ? b[0] : 0;
}

void main() {
  gl_Position =
    uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aVertex, 1.);
  gl_Position.x += Unused(aNormal, aTexCoords);
  gl_Position.z += uZOffset;
}
