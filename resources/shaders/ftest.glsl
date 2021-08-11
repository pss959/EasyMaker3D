varying vec3 vPosition;
varying vec2 vTexCoords;
varying vec3 vNormal;
varying vec3 vToLight0;
varying vec3 vToLight1;

uniform vec4      uMyColor = vec4(1, 0, 0, 1);
uniform sampler2D uMyTexture;

void main(void) {
  float intensity = .8;
  gl_FragColor = intensity * texture2D(uMyTexture, vTexCoords);
}
