#version 330 core

#define MAX_LIGHTS 4

uniform vec4             uBaseColor;
uniform float            uAmbientIntens;
uniform int              uLightCount;
uniform int              uShowTexture;
uniform vec2             uTextureScale;
uniform sampler2D        uTexture;

// Per-light uniforms:
uniform vec3             uLightPos[MAX_LIGHTS];
uniform vec4             uLightColor[MAX_LIGHTS];
uniform sampler2DShadow  uShadowMap[MAX_LIGHTS];

in vec3 vWorldPos;     // Vertex position in world coordinates.
in vec3 vWorldNormal;  // Normal in world coordinates.
in vec2 vTexCoords;    // Texture coordinates.

// Per-light attributes:
in vec4 vShadowPos[MAX_LIGHTS];       // Shadow positions.

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

float GetShadowVisibility(vec4 shadow_pos, sampler2DShadow shadow_map) {
  // Fixed bias, or...
  float bias = 0.005;

  // ...variable bias
  // float bias = 0.005*tan(acos(cosTheta));
  // bias = clamp(bias, 0,0.01);

  float visibility = 1.0;

  // Sample the shadow map 4 times
  for (int i = 0; i < 4; ++i) {
    // use either :
    //  - Always the same samples.
    //    Gives a fixed pattern in the shadow, but no noise
    int index = i;
    //  - A random sample, based on the pixel's screen location.
    //    No banding, but the shadow moves with the camera, which looks weird.
    // int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
    //  - A random sample, based on the pixel's position in world space.
    //    The position is rounded to the millimeter to avoid too much aliasing
    // int index = int(16.0*random(floor(Position_worldspace.xyz*1000.0), i))%16;

    // being fully in the shadow will eat up 4*0.2 = 0.8
    // 0.2 potentially remain, which is quite dark.

    vec3 tc = vec3(shadow_pos.xy + poissonDisk[index] / 700.0,
                   (shadow_pos.z - bias) / shadow_pos.w);
    visibility -= .2 * (1. - texture(shadow_map, tc));
  }
  return visibility;
}

void main(void) {
  vec3 n = normalize(vWorldNormal);

  vec4 result = vec4(uAmbientIntens);

  for (int i = 0; i < uLightCount; ++i) {
    vec3 to_light = uLightPos[i] - vWorldPos;
    float ldotn = max(0., dot(normalize(to_light), n));
    vec4 diffuse = uLightColor[i] * uBaseColor * ldotn;
    //float visibility = GetShadowVisibility(vShadowPos[i], uShadowMap[i]);
    float visibility = .5; // XXXX
    result += visibility * diffuse;
  }

  if (uShowTexture != 0)
    result *= texture2D(uTexture, uTextureScale * vTexCoords);

  gl_FragColor = result;
}
