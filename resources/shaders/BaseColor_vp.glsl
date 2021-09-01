#version 330 core

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoords;

// Hack to shut up warnings about unused attributes.
float Unused(vec3 a, vec2 b) {
  return a.x * b.x - b.x * a.x;
}

void main() {
  gl_Position =
    uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aVertex, 1.);
  gl_Position.x += Unused(aNormal, aTexCoords);
}
