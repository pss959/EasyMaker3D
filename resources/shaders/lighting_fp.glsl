#version 330 core

#define MAX_LIGHTS 4

uniform vec4             uBaseColor;
uniform float            uAmbientIntens;
uniform int              uLightCount;
uniform int              uShowTexture;
uniform sampler2D        uTexture;

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

// XXXX
vec2 poissonDisk[16] = vec2[](
   vec2( -0.94201624, -0.39906216 ),
   vec2( 0.94558609, -0.76890725 ),
   vec2( -0.094184101, -0.92938870 ),
   vec2( 0.34495938, 0.29387760 ),
   vec2( -0.91588581, 0.45771432 ),
   vec2( -0.81544232, -0.87912464 ),
   vec2( -0.38277543, 0.27676845 ),
   vec2( 0.97484398, 0.75648379 ),
   vec2( 0.44323325, -0.97511554 ),
   vec2( 0.53742981, -0.47373420 ),
   vec2( -0.26496911, -0.41893023 ),
   vec2( 0.79197514, 0.19090188 ),
   vec2( -0.24188840, 0.99706507 ),
   vec2( -0.81409955, 0.91437590 ),
   vec2( 0.19984126, 0.78641367 ),
   vec2( 0.14383161, -0.14100790 )
);

float GetShadowVisibility(vec4 light_vertex_pos, sampler2D shadow_map) {
  // Get the closest depth from the shadow map in (-1,1) and convert to (0, 1).
  vec3 coords = .5 + .5 * (light_vertex_pos.xyz / light_vertex_pos.w);
  float closest_depth = texture(shadow_map, coords.xy).r;

  // Get the depth of this fragment.
  float frag_depth = coords.z;

  return frag_depth >= closest_depth ? 1.0 : 0.0;
}

void main(void) {
  // Do all lighting computations in world coordinates.
  vec3 n = normalize(vWorldNormal);

  result_color = vec4(uAmbientIntens);

  for (int i = 0; i < uLightCount; ++i) {
    vec3 to_light = uLightPos[i] - vWorldVertex;
    float ldotn = max(0., dot(normalize(to_light), n));
    vec4 diffuse = uLightColor[i] * uBaseColor * ldotn;
    float visibility = GetShadowVisibility(vLightVertexPos[i],
                                           uLightShadowMap[i]);

    float diff_scale = .6;  // XXXX
    result_color += visibility * diff_scale * diffuse;
  }

  if (uShowTexture != 0)
    result_color *= texture2D(uTexture, vScaledTexCoords);
}
