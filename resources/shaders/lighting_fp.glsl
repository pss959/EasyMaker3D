uniform float            uAmbientIntens;
uniform vec4             uBaseColor;
uniform vec3             uLight0Dir;
uniform vec3             uLight1Dir;
uniform vec3             uLight2Dir;
uniform float            uLight0Intens;
uniform float            uLight1Intens;
uniform float            uLight2Intens;
uniform int              uShowTexture;
uniform sampler2D        uTexture;
uniform sampler2DShadow  uShadowMap;
uniform vec2             uTextureScale;

varying vec3 vNormal;
varying vec3 vPosition;
varying vec2 vTexCoords;
varying vec4 vShadowPos;

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

float GetLightDiffuse(vec3 dir, float intens, vec3 n) {
  return intens * max(0., dot(-normalize(dir), n));
}

void main(void) {
  vec3 n = normalize(vNormal);
  float diffuse0 = GetLightDiffuse(uLight0Dir, uLight0Intens, n);
  float diffuse1 = GetLightDiffuse(uLight1Dir, uLight1Intens, n);
  float diffuse2 = GetLightDiffuse(uLight2Dir, uLight2Intens, n);

  float visibility = 1.0;

  // Fixed bias, or...
  float bias = 0.005;

  // ...variable bias
  // float bias = 0.005*tan(acos(cosTheta));
  // bias = clamp(bias, 0,0.01);

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

    vec3 tc = vec3(vShadowPos.xy + poissonDisk[index] / 700.0,
                   (vShadowPos.z - bias) / vShadowPos.w);
    visibility -= .2 * (1. - texture(uShadowMap, tc));
  }

  float intensity = min(1., uAmbientIntens + (visibility * diffuse0) +
                        diffuse1 + diffuse2);

  gl_FragColor = intensity * uBaseColor;
  if (uShowTexture != 0)
      gl_FragColor *= texture2D(uTexture, uTextureScale * vTexCoords);
}
