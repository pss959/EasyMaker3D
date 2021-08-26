#version 330 core

uniform mat4 uProjectionMatrix;  // Relative to light source.
uniform mat4 uModelviewMatrix;   // Relative to light source.

in vec3 aVertex;
in vec3 aNormal;
in vec3 aTexCoords;

// Hack to shut up warnings about unused attributes.
float Unused(vec3 a, vec3 b) {
  return 0.00000000001 * (a.x + b.x);
}

void main() {
  gl_Position = uProjectionMatrix * uModelviewMatrix * vec4(aVertex, 1.);
}
