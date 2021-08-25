#version 330 core

#define MAX_LIGHTS 4

uniform mat4 uProjectionMatrix;
uniform mat4 uModelviewMatrix;
uniform int  uLightCount;

// Per-light uniforms:
uniform mat4 uBiasMatrix[MAX_LIGHTS];

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoords;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoords;

// Per-light attributes:
out vec4 vShadowPos[MAX_LIGHTS];

void main(void) {
  vec4 vertex = vec4(aVertex, 1.);

  vPosition  = aVertex;
  vNormal    = aNormal;
  vTexCoords = aTexCoords;

  for (int i = 0; i < uLightCount; ++i)
    vShadowPos[i] = uBiasMatrix[i] * vertex;

  gl_Position = uProjectionMatrix * uModelviewMatrix * vertex;
}
