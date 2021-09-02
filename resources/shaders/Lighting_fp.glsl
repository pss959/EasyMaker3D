#version 330 core

#define MAX_LIGHTS 4

uniform vec3       uViewPos;
uniform vec4       uBaseColor;
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

// Returns 0 if in total shadow, 1 if not in shadow at all.
float GetShadowVisibility(vec4 light_vertex_pos, sampler2D shadow_map) {
  // If this object does not receive shadows, use full visibility.
  if (uReceiveShadows == 0)
    return 1.;

  // Get the closest depth from the shadow map in (-1,1) and convert to (0, 1).
  vec3 coords = .5 + .5 * (light_vertex_pos.xyz / light_vertex_pos.w);
  float closest_depth = texture(shadow_map, coords.xy).r;

  // Get the depth of this fragment.
  float frag_depth = coords.z;

  return frag_depth > closest_depth ? 0. : 1.;
}

// Square function for convenience.
float Square(float n) { return n * n; }

// Fresnel approximation function from Strauss lighting model.
float F(float x) {
  float kf  = 1.12;
  float inv_kf2 = 1. / Square(kf);
  float d1 = 1.0 - kf;
  return (1. / Square(x - kf) - inv_kf2) / (1. / Square(1 - kf) - inv_kf2);
}

// Geometric attenuation function from Strauss lighting model.
float G(float x) {
  float kg = 1.01;
  float inv_kg2     = 1. / Square(kg);
  float inv_neg_kg2 = 1. / Square(1 - kg);
  return (inv_neg_kg2 - 1. / Square(x - kg)) / (inv_neg_kg2 - inv_kg2);
}

vec4 Light(vec3 normal, vec3 view_vec, vec3 light_vec,
           vec4 light_color, vec4 tex_color) {
  vec3 surf_color = uBaseColor.rgb * tex_color.rgb;
  float opacity = uBaseColor.a;

  vec3 H = reflect(light_vec, normal);

  // Cosines of angles.
  float alpha = -dot(normal, light_vec);  // Incident angle.
  float beta  = -dot(H,      view_vec);   // Specular angle.
  float gamma = -dot(normal, view_vec);   // View angle.

  // Diffuse contribution.
  float rd = (1. - uSmoothness * uSmoothness * uSmoothness) * opacity;
  float d  = 1. - uMetalness * uSmoothness;
  vec3 diffuse = alpha * d * rd * surf_color.rgb;

  // Specular contribution.
  float kj = .1;
  float h = 3. / (1. - uSmoothness);
  float rn = opacity - rd;
  float f = F(alpha);
  float j = f * G(alpha) * G(gamma);
  float rj = min(1, rn + (rn + kj) * j);
  float rs = pow(max(0, beta), h) * rj;

  vec3 C1 = vec3(1, 1, 1);
  vec3 spec_color = C1 + uMetalness * (1. - f) * (surf_color - C1);
  vec3 specular = rs * spec_color;

  vec3 lit_color = light_color.rgb * (diffuse + specular);
  return vec4(lit_color, uBaseColor.a);
}

void main(void) {
  vec4 tex_color = uShowTexture != 0 ?
    texture2D(uTexture, vScaledTexCoords) : vec4(1, 1, 1, 1);

  // Do all lighting computations in world coordinates.
  vec3 normal   = normalize(vWorldNormal);
  vec3 view_vec = normalize(vWorldVertex - uViewPos);

  result_color = vec4(uAmbientIntens);

  // Scale each light source's contribution based on the number of sources.
  float light_scale = 1. / uLightCount;

  for (int i = 0; i < uLightCount; ++i) {
    vec3 light_vec = normalize(vWorldVertex - uLightPos[i]);
    float vis = GetShadowVisibility(vLightVertexPos[i], uLightShadowMap[i]);
    vec4 refl = Light(normal, view_vec, light_vec, uLightColor[i], tex_color);
    result_color += light_scale * vis * refl;
  }
}
