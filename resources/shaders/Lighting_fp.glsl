#version 330 core

#define MAX_LIGHTS 4

uniform vec3       uViewPos;
uniform vec4       uBaseColor;
uniform vec4       uEmissiveColor;
uniform float      uSmoothness;
uniform float      uMetalness;
uniform float      uAmbientIntens;
uniform int        uLightCount;
uniform int        uShowTexture;
uniform int        uReceiveShadows;
uniform sampler2D  uTexture;

// Per-light uniforms:
uniform vec3       uLightPos[MAX_LIGHTS];     // Position in world coords.
uniform vec4       uLightColor[MAX_LIGHTS];
uniform sampler2D  uLightShadowMap[MAX_LIGHTS];

in vec3 vWorldVertex;                 // Vertex position in world coordinates.
in vec3 vWorldNormal;                 // Normal in world coordinates.
in vec2 vScaledTexCoords;             // Texture coordinates.

// Per-light attributes:
in vec4 vLightVertexPos[MAX_LIGHTS];  // Vertex positions relative to lights.

out vec4 result_color;

@include "LightShadowing_inc.fp"
@include "Lighting_inc.fp"

void main(void) {
  // Do all lighting computations in world coordinates.
  ldata.base_color = uBaseColor;
  ldata.tex_color  = uShowTexture != 0 ? texture2D(uTexture, vScaledTexCoords) :
    vec4(1, 1, 1, 1);
  ldata.smoothness = uSmoothness;
  ldata.metalness  = uMetalness;
  ldata.normal     = normalize(vWorldNormal);
  ldata.view_vec   = normalize(vWorldVertex - uViewPos);

  result_color = vec4(uAmbientIntens) + uEmissiveColor;
  for (int i = 0; i < uLightCount; ++i) {
    ldata.light_vec   = normalize(vWorldVertex - uLightPos[i]);
    ldata.light_color = uLightColor[i];
    float vis = uReceiveShadows == 0 ? 1. :
      GetShadowVisibility(vLightVertexPos[i], uLightShadowMap[i]);
    result_color += vis * Light(ldata);
  }
}
