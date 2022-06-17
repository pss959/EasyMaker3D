#version 330 core

uniform vec4      uBaseColor;
uniform vec4      uEmissiveColor;
uniform int       uShowTexture;
uniform sampler2D uTexture;

in vec3 vWorldVertex;      // Vertex position in world coordinates.
in vec3 vWorldNormal;      // Normal in world coordinates.
in vec2 vScaledTexCoords;  // Scaled texture coordinates.

out vec4 result_color;

void main(void) {
  vec3 ambient = vec3(.2, .2, .2);
  float intensity = abs(normalize(vWorldNormal).z);
  if (uShowTexture == 0) {
    result_color.rgb = intensity * uBaseColor.rgb;
    result_color.a   = uBaseColor.a;
  }
  else {
    vec4 tex_color = texture2D(uTexture, vScaledTexCoords);
    result_color.rgb = intensity * tex_color.rgb;
    result_color.a   = tex_color.a * uBaseColor.a;
  }
  result_color.rgb += ambient + uEmissiveColor.rgb;
}
