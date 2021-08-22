varying vec2 vTexCoords;

uniform sampler2D uTexture;
uniform vec2      uTextureScale;
uniform vec4      uBaseColor;

void main(void) {
  float intensity = .8;
  gl_FragColor = intensity * uBaseColor *
    texture2D(uTexture, uTextureScale * vTexCoords);
}
