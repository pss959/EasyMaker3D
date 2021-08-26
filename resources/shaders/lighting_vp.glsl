#version 330 core

#define MAX_LIGHTS 4

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelviewMatrix;
uniform vec2 uTextureScale;
uniform int  uLightCount;

// Per-light uniforms:
uniform vec3 uLightPos[MAX_LIGHTS];       // Position in world coords.
uniform mat4 uLightMatrix[MAX_LIGHTS];    // Light's proj * view matrices.

in vec3 aVertex;        // Vertex position in object coordinates.
in vec3 aNormal;        // Normal in object coordinates.
in vec2 aTexCoords;     // Texture coordinates.

out vec3 vEyeVertex;    // Vertex position in eye coordinates.
out vec3 vEyeNormal;    // Normal in eye coordinates.
out vec2 vTexCoords;    // Texture coordinates.

// Per-light attributes:
out vec3 vEyeLightPos[MAX_LIGHTS];     // Light position in eye coordinates.
out vec4 vLightVertexPos[MAX_LIGHTS];  // Vertex position relative to light.

void main(void) {
  // Vertex in object coordinates.
  vec4 obj_vertex = vec4(aVertex, 1.);

  // Convert vertex to eye coordinates by modelview matrix.
  vec4 eye_vertex = uModelviewMatrix * obj_vertex;
  vEyeVertex = eye_vertex.xyz;

  // Convert normal to eye coordinates with the modelview inverse transpose.
  vEyeNormal = transpose(inverse(mat3(uModelviewMatrix))) * aNormal;

  // Scale texture coordinates.
  vTexCoords = uTextureScale * aTexCoords;

  // Convert light positions to eye coordinates by pure view matrix. Convert
  // shadow positions by the bias matrix.
  for (int i = 0; i < uLightCount; ++i) {
    vEyeLightPos[i]    = (uViewMatrix * vec4(uLightPos[i], 1)).xyz;
    vLightVertexPos[i] = uLightMatrix[i] * obj_vertex;
  }

  // The vertex position requires applying the projection matrix.
  gl_Position = uProjectionMatrix * eye_vertex;
}
