uniform float uAmbientIntens;
uniform vec4  uBaseColor;
uniform vec3  uLight0Dir;
uniform vec3  uLight1Dir;
uniform float uLight0Intens;
uniform float uLight1Intens;

varying vec3 vNormal;

void main(void) {
  vec3 n = normalize(vNormal);
  float diffuse0 = uLight0Intens * max(0., dot(-normalize(uLight0Dir), n));
  float diffuse1 = uLight1Intens * max(0., dot(-normalize(uLight1Dir), n));
  float intensity = min(1., uAmbientIntens + diffuse0 + diffuse1);
  gl_FragColor = intensity * uBaseColor;
}
