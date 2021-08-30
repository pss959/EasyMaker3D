#version 330 core

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

in vec3 aVertex;

void main() {
  gl_Position =
    uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aVertex, 1.);
}
