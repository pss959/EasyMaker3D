#version 330 core

#define MAX_LIGHTS 4

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform int  uLightCount;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// Input from the vertex program.
in VertexOutput {
  vec3 obj_pos;                // Position in object coordinates.
  vec3 world_pos;              // Position in world coordinates.
  vec4 light_pos[MAX_LIGHTS];  // Position relative to each light.
} geom_input[];

// Output from the geometry shader.
out GeometryOutput {
  vec3 obj_pos;                // Position in object coordinates.
  vec3 world_pos;              // Position in world coordinates.
  vec3 world_normal;           // Face normal in world coordinates.
  vec3 barycentric;            // Barycentric coordinates of the vertex.
  vec4 light_pos[MAX_LIGHTS];  // Position relative to each light.
} geom_output;

void main() {
  // Compute the triangle normal in world coordinates.
  vec3 p0 = geom_input[0].world_pos;
  vec3 p1 = geom_input[1].world_pos;
  vec3 p2 = geom_input[2].world_pos;
  vec3 normal = normalize(cross(p1 - p0, p2 - p0));

  // Barycentric coordinates for each vertex.
  vec3 barycentric[3] = vec3[]( vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1) );

  // Output each vertex of the triangle.
  for (int i = 0; i < 3; ++i) {
    geom_output.obj_pos      = geom_input[i].obj_pos;
    geom_output.world_pos    = geom_input[i].world_pos;
    geom_output.world_normal = normal;
    geom_output.barycentric  = barycentric[i];
    for (int j = 0; j < uLightCount; ++j)
      geom_output.light_pos[j] = geom_input[i].light_pos[j];
    gl_Position =
      uProjectionMatrix * uViewMatrix * vec4(geom_output.world_pos, 1);
    EmitVertex();
  }
  EndPrimitive();
}
