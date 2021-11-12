#version 330 core

#define MAX_LIGHTS 4

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform vec2 uTextureScale;
uniform vec2 uTextureOffset;
uniform int  uLightCount;

// Per-light uniforms:
uniform mat4 uLightMatrix[MAX_LIGHTS];  // Light's proj * view matrices.

in vec3 aVertex;            // Vertex position in object coordinates.
in vec3 aNormal;            // Normal in object coordinates.
in vec2 aTexCoords;         // Texture coordinates.

out vec3 vWorldVertex;      // Vertex position in world coordinates.
out vec3 vWorldNormal;      // Normal in world coordinates.
out vec2 vScaledTexCoords;  // Scaled texture coordinates.

// Per-light attributes:
out vec4 vLightVertexPos[MAX_LIGHTS];  // Vertex position relative to light.

void main(void) {
  // Vertex in object and world coordinates.
  vec4 obj_vertex = vec4(aVertex, 1.);
  vec4 world_pos  = uModelMatrix * obj_vertex;

  vWorldVertex     = world_pos.xyz;
  vWorldNormal     = transpose(inverse(mat3(uModelMatrix))) * aNormal;
  vScaledTexCoords = uTextureScale * aTexCoords - uTextureOffset;

  // Compute the vertex position relative to each light.
  for (int i = 0; i < uLightCount; ++i)
    vLightVertexPos[i] = uLightMatrix[i] * world_pos;

  gl_Position = uProjectionMatrix * uViewMatrix * world_pos;
}
