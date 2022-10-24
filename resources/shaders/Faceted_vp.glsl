#version 330 core

#define MAX_LIGHTS 4

uniform mat4 uModelMatrix;
uniform int  uLightCount;
uniform mat4 uLightMatrix[MAX_LIGHTS];  // Each light's proj * view matrices.

layout (location = 0) in vec3 aVertex;

out VertexOutput {
  vec3 obj_pos;                // Position in object coordinates.
  vec3 world_pos;              // Position in world coordinates.
  vec4 light_pos[MAX_LIGHTS];  // Position relative to each light.
} vertex_output;

void main() {
  vertex_output.obj_pos = aVertex;

  // Vertex position in world coordinates.
  vec4 world_pos = uModelMatrix * vec4(aVertex, 1);
  vertex_output.world_pos = world_pos.xyz;

  // Compute the vertex position relative to each light.
  for (int i = 0; i < uLightCount; ++i)
    vertex_output.light_pos[i] = uLightMatrix[i] * world_pos;
}
