#version 330 core

#define MAX_LIGHTS 4

uniform mat4 uProjectionMatrix;
uniform mat4 uModelviewMatrix;
uniform mat3 uNormalMatrix;
uniform int  uLightCount;

// Per-light uniforms:
uniform mat4 uBiasMatrix[MAX_LIGHTS];  // Matrix applied to shadow positions.

in vec3 aVertex;        // Vertex position in object coordinates.
in vec3 aNormal;        // Normal in object coordinates.
in vec2 aTexCoords;     // Texture coordinates.

out vec3 vWorldPos;     // Vertex position in world coordinates.
out vec3 vWorldNormal;  // Normal in world coordinates.
out vec2 vTexCoords;    // Texture coordinates.

// Per-light attributes:
out vec4 vShadowPos[MAX_LIGHTS];       // Shadow positions.

void main(void) {
  vec4 obj_pos   = vec4(aVertex, 1.);
  vec4 world_pos = uProjectionMatrix * uModelviewMatrix * obj_pos;

  vWorldPos    = world_pos.xyz;
  vWorldNormal = uNormalMatrix * aNormal;
  vTexCoords   = aTexCoords;

  for (int i = 0; i < uLightCount; ++i)
    vShadowPos[i] = uBiasMatrix[i] * obj_pos;

  gl_Position = world_pos;
}
