varying vec3 vNormal;
varying vec2 vTexCoords;

uniform float     uAmbientIntens;
uniform vec4      uBaseColor;
uniform vec3      uLight0Dir;
uniform vec3      uLight1Dir;
uniform vec3      uLight2Dir;
uniform float     uLight0Intens;
uniform float     uLight1Intens;
uniform float     uLight2Intens;
uniform int       uShowTexture;
uniform sampler2D uTexture;
uniform vec2      uTextureScale;

float GetLightDiffuse(vec3 dir, float intens, vec3 n) {
  return intens * max(0., dot(-normalize(dir), n));
}

void main(void) {
  vec3 n = normalize(vNormal);
  float diffuse0 = GetLightDiffuse(uLight0Dir, uLight0Intens, n);
  float diffuse1 = GetLightDiffuse(uLight1Dir, uLight1Intens, n);
  float diffuse2 = GetLightDiffuse(uLight2Dir, uLight2Intens, n);
  float intensity = min(1., uAmbientIntens + diffuse0 + diffuse1 + diffuse2);
  gl_FragColor = intensity * uBaseColor;
  if (uShowTexture != 0)
      gl_FragColor *= texture2D(uTexture, uTextureScale * vTexCoords);
}
