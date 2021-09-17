#version 330 core

#define MAX_LIGHTS 4

// Constants for all faceted models.
float kSmoothness    = .2;
float kMetalness     = 0;
float kAmbientIntens = .2;
vec4  kEmissiveColor = vec4(0, 0, 0, 0);
int   kReceiveShadows = 1;

uniform vec3       uViewPos;
uniform vec4       uBaseColor;
uniform int        uLightCount;
uniform int        uShowTexture;
uniform sampler2D  uTexture;

// Per-light uniforms:
uniform vec3       uLightPos[MAX_LIGHTS];     // Position in world coords.
uniform vec4       uLightColor[MAX_LIGHTS];
uniform sampler2D  uLightShadowMap[MAX_LIGHTS];

// Input from geometry shader.
in GeometryOutput {
  vec3 world_pos;              // Position in world coordinates.
  vec3 world_normal;           // Face normal in world coordinates.
  vec3 barycentric;            // Barycentric coordinates of the vertex.
  vec4 light_pos[MAX_LIGHTS];  // Position relative to each light.
} frag_input;

out vec4 result_color;

@include "LightShadowing_inc.fp"
@include "Lighting_inc.fp"

void main(void) {
  // Do all lighting computations in world coordinates.
  ldata.base_color = uBaseColor;
  // No textures for this shader.
  ldata.tex_color  = vec4(1, 1, 1, 1);
  ldata.smoothness = kSmoothness;
  ldata.metalness  = kMetalness;
  ldata.normal     = frag_input.world_normal;
  ldata.view_vec   = normalize(frag_input.world_pos - uViewPos);

  result_color = vec4(kAmbientIntens) + kEmissiveColor;
  for (int i = 0; i < uLightCount; ++i) {
    ldata.light_vec   = normalize(frag_input.world_pos - uLightPos[i]);
    ldata.light_color = uLightColor[i];
    float vis = GetShadowVisibility(frag_input.light_pos[i],
                                    uLightShadowMap[i]);
    result_color += vis * Light(ldata);
  }
}
