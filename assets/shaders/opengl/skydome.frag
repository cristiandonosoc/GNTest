#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes ------------------------------------------------------------------

out vec4 out_color;

// Uniforms --------------------------------------------------------------------

layout (std140) uniform FragUniforms {
  ivec2 resolution;
  vec3 sky_color1;
  vec3 sky_color2;
} uniforms;

// Code ------------------------------------------------------------------------

#define FOCAL_LENGTH 0.9
#define MAX_DIST 32

float pi = atan(1.0) * 4.0;
float tau = 2 * pi;

// Returns a rotation matrix for the given angles around the X,Y,Z axes.
mat3 Rotate(vec3 angles) {
  vec3 c = cos(angles);
  vec3 s = sin(angles);

  mat3 rotX = mat3( 1.0,  0.0,  0.0,
                    0.0,  c.x,  s.x,
                    0.0, -s.x,  c.x);

  mat3 rotY = mat3( c.y,  0.0, -s.y,
                    0.0,  1.0,  0.0,
                    s.y,  0.0,  c.y);

  mat3 rotZ = mat3( c.z,  s.z,  0.0,
                   -s.z,  c.z,  0.0,
                    0.0,  0.0,  1.0);
        //object = opU(object, -sdSphere(dir * dist, MAX_DIST, SKYDOME));

  return rotX * rotY * rotZ;
}

struct RaycastHit {
  vec3 position;
};

RaycastHit Raycast(vec3 origin, vec3 dir) {
  // We only have a sphere that contains our background.

  RaycastHit hit;
  hit.position = origin + dir * MAX_DIST;
  return hit;
}

vec3 Shade(RaycastHit hit, vec3 direction) {
  vec3 color = vec3(0);
  color = mix(uniforms.sky_color1 * 1.4,
              uniforms.sky_color2,
              -hit.position / 7.0);
  return color;
}

void main() {
  vec2 res = uniforms.resolution.xy / uniforms.resolution.y;
  vec2 uv = gl_FragCoord.xy / uniforms.resolution.y;

  vec3 angles = vec3(0);
  angles.y = tau * (1.8 / 8.0);
  angles.x = tau * (1.8 / 8.0) * 1.1;
  angles.y = clamp(angles.y, 0.0, 15.5 * tau / 64.0);

  mat3 rotate = Rotate(angles.yzx);

  vec3 origin = vec3(0, 1, 0) * rotate;
  vec3 dir = normalize(vec3(uv - res / 2.0, FOCAL_LENGTH)) * rotate;
  origin.z += 0.25;

  RaycastHit hit = Raycast(origin, dir);
  vec3 color = Shade(hit, dir);

  out_color = vec4(color, 1.0f);


  /* if (gl_FragCoord.y < 200) */
  /*   color = vec4(1, 0, 0, 1); */
  /* else */
  /*   color = vec4(0, 1, 0, 1); */

  /* float i = gl_FragCoord.x / uniforms.screen_size.x; */
  // color = vec4(i * uniforms.sky_color1 + (1 - i) * uniforms.sky_color2, 1.0f);
  // color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
}
