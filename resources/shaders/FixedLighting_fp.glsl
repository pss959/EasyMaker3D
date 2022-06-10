#version 330 core

uniform sampler2D uTexture;

in vec3 vWorldVertex;  // Vertex position in world coordinates.
in vec3 vWorldNormal;  // Normal in world coordinates.
in vec2 vTexCoords;    // Texture coordinates.

out vec4 result_color;

void main(void) {
  vec4 tex_color  = texture2D(uTexture, vTexCoords);
  float intensity = abs(normalize(vWorldNormal).z);
  result_color = intensity * tex_color;
}
