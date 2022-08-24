#version 330 core

uniform mat4 uLightMatrix;  // proj * view for light source.
uniform mat4 uModelMatrix;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoords;

// Hack to shut up warnings about unused attributes.
float Unused(vec3 a, vec2 b) {
  return (100 * a[0]) < -1000 ? b[0] : 0;
}

void main() {
  gl_Position = uLightMatrix * uModelMatrix * vec4(aVertex, 1.);
  gl_Position.x += Unused(aNormal, aTexCoords);
}
