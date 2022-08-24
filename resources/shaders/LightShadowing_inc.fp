// This can be included in a fragment shader to compute shadow visibility.

// XXXX Improve this?
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
  // Get the closest depth from the shadow map in (-1,1) and convert to (0, 1).
  vec3 coords = .5 + .5 * (light_vertex_pos.xyz / light_vertex_pos.w);

  // Sample the shadow map several times
  int   kSamples = 8;
  float kJitterScale = .002;
  float vis = 0;
  for (int i = 0; i < kSamples; ++i) {
    vec2 adj_coords = coords.xy + kJitterScale * poissonDisk[i];

    float closest_depth = texture(shadow_map, adj_coords).r;

    // Compare the depth of this fragment against the depth from the map.
    vis += coords.z > closest_depth ? 0. : 1.;
  }
  return vis / kSamples;
}
