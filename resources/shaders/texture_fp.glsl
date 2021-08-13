varying vec3 vPosition;
varying vec2 vTexCoords;

uniform sampler2D uTexture;
uniform vec2      uTextureScale;

void main(void) {
  float intensity = .8;
  gl_FragColor = intensity * texture2D(uTexture, uTextureScale * vTexCoords);
}
