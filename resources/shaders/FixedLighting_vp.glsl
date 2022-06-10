#version 330 core

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

in vec3 aVertex;        // Vertex position in object coordinates.
in vec3 aNormal;        // Normal in object coordinates.
in vec2 aTexCoords;     // Texture coordinates.

out vec3 vWorldVertex;  // Vertex position in world coordinates.
out vec3 vWorldNormal;  // Normal in world coordinates.
out vec2 vTexCoords;    // Scaled texture coordinates.

void main() {
  // Vertex in object and world coordinates.
  vec4 obj_vertex = vec4(aVertex, 1.);
  vec4 world_pos  = uModelMatrix * obj_vertex;

  vWorldVertex = world_pos.xyz;
  vWorldNormal = transpose(inverse(mat3(uModelMatrix))) * aNormal;
  vTexCoords   = aTexCoords;

  gl_Position = uProjectionMatrix * uViewMatrix * world_pos;
}
