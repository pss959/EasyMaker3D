// This can be included in a fragment shader to compute lighting.

// This is passed to the Light() function.
struct LightingData {
  vec4  base_color;
  vec4  tex_color;
  float smoothness;
  float metalness;
  vec3  normal;
  vec3  view_vec;
  vec3  light_vec;
  vec4  light_color;
} ldata;

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

vec4 Light(LightingData ldata) {
  vec3 surf_color = ldata.base_color.rgb * ldata.tex_color.rgb;
  float opacity = ldata.base_color.a;

  vec3 H = reflect(ldata.light_vec, ldata.normal);

  // Cosines of angles.
  float alpha = -dot(ldata.normal, ldata.light_vec);  // Incident angle.
  float beta  = -dot(H,            ldata.view_vec);   // Specular angle.
  float gamma = -dot(ldata.normal, ldata.view_vec);   // View angle.

  // Diffuse contribution.
  float s3 = ldata.smoothness * ldata.smoothness * ldata.smoothness;
  float rd = (1. - s3) * opacity;
  float d  = 1. - ldata.metalness * ldata.smoothness;
  vec3 diffuse = alpha * d * rd * surf_color.rgb;

  // Specular contribution.
  float kj = .1;
  float h = 3. / (1. - ldata.smoothness);
  float rn = opacity - rd;
  float f = F(alpha);
  float j = f * G(alpha) * G(gamma);
  float rj = min(1, rn + (rn + kj) * j);
  float rs = pow(max(0, beta), h) * rj;

  vec3 C1 = vec3(1, 1, 1);
  vec3 spec_color = C1 + ldata.metalness * (1. - f) * (surf_color - C1);
  vec3 specular = rs * spec_color;

  vec3 lit_color = ldata.light_color.rgb * (diffuse + specular);
  return vec4(lit_color, ldata.base_color.a);
}
