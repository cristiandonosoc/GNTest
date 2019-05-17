#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes ------------------------------------------------------------------

out vec4 out_color;

// Uniforms --------------------------------------------------------------------

layout (std140) uniform FragUniforms {
  ivec2 iResolution;
  vec3 sky_color1;
  vec3 sky_color2;
} ;

// Code ------------------------------------------------------------------------

#define MAX_DIST 30.0f

#define BLACK vec3(0, 0, 0)
#define WHITE vec3(1, 1, 1)
#define RED vec3(1, 0, 0)
#define GREEN vec3(0, 1, 0)
#define BLUE vec3(0, 0, 1)
#define YELLOW vec3(1, 1, 0)

struct RaycastHit {
  vec3 position;
};

RaycastHit Raycast(vec3 origin, vec3 dir) {
  // We only have a sphere that contains our background.

  RaycastHit hit;
  hit.position = origin + dir * MAX_DIST;
  return hit;
}
vec3 CheckerBoard(vec3 hit) {
  if (abs(hit.x) < 0.05)
      return YELLOW;
  if (abs(hit.y) < 0.05)
      return YELLOW;
  if ((int(floor(hit.x)) + int(floor(hit.y))) % 2 == 1) {
    // Positive is blue, negative is red.
      if (hit.y >= 0.0f) {
        return mix(WHITE, BLUE, hit.y / MAX_DIST);
      } else {
        return mix(WHITE, RED, -hit.y / MAX_DIST);
      }


  } else {
    return BLACK;
  }
}

vec3 Shade(RaycastHit hit, vec3 direction) {
  vec3 color = vec3(0);

  return CheckerBoard(hit.position);
}


void mainImage(out vec4 fragColor, in vec2 fragCoord) {
  vec2 res = iResolution.xy / iResolution.y;
  vec2 uv = fragCoord.xy / iResolution.y;

  float aspect = iResolution.x / iResolution.y;

  float o = 2.0f;
  vec3 origin = vec3(0, 0, o);
  vec3 dir = normalize(vec3(uv - res / 2.0, -o));

  RaycastHit hit = Raycast(origin, dir);
  vec3 color = Shade(hit, dir);

  fragColor = vec4(color, 1.0f);
}

void main() {
  vec2 uv = gl_FragCoord.xy / iResolution;
  out_color = vec4(uv, 0, 1.0f);


  /* vec2 uv = gl_FragCoord.xy; */
  /* mainImage(out_color, uv); */
}

