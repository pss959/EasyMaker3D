#version 330 core

uniform mat4 uShadowMatrix;
uniform vec3 uLightPos;

// Note the normal/texcoords code is just to shut up Ion warnings about unused
// attribute.
in vec3 aVertex;
in vec3 aNormal;
in vec3 aTexCoords;

out vec3 vToLight;

float Unused(vec3 a, vec3 b) {
  return 0.00000000001 * (a.x + b.x);
}

void main() {
  vToLight    = uLightPos - aVertex;
  gl_Position = uShadowMatrix * vec4(aVertex, 1.);
  gl_Position.x += Unused(aNormal, aTexCoords);
}
