/*
MADE BY MORGAN VILLEDIEU
TW : https://twitter.com/VilledieuMorgan
Click and drag to rotate the camera
*/

// Raymarch settings

#define MAX_MOVEMENT_SPEED 0.9
#define MIN_RADIUS 0.01
#define MAX_RADIUS 0.3
#define STAR_COUNT 30
#define PI 3.14159265358979323
#define TWOPI 6.283185307

#define EPSILON 0.1

#define RADIUS_SEED 1337.0
#define START_POS_SEED 2468.0
#define THETA_SEED 1675.0

#define MIN_DIST 0.001
#define MAX_DIST 32.0
#define MAX_STEPS 96
#define STEP_MULT 0.9
#define NORMAL_OFFS 0.01
#define FOCAL_LENGTH 0.9

// Scene settings

//#define SHOW_RAY_COST

// Colors
#define SKY_COLOR_1 vec3(49., 33., 66.) / 255.
#define SKY_COLOR_2 vec3(0.00, 0.05, 0.20)

#define SUN_COLOR_2 vec3(87., 33., 73.) / 255.
#define SUN_COLOR_1 vec3(1.00, 0.20, 0.60) / 2.

#define GRID_COLOR_1 vec3(0.00, 0.05, 0.20)
#define GRID_COLOR_2 vec3(26.00, 14.0, 122.0) / 255.
#define FOG_COLOR vec3(193.00, 24.0, 123.0) / 255.

// Parameters
#define GRID_SIZE 0.50
#define GRID_LINE_SIZE 1.25

#define SUN_DIRECTION vec3(0.10, 0.0, 0.)

#define CLOUD_SCROLL vec2(0.002, 0.001)
#define CLOUD_BLUR 2.0
#define CLOUD_SCALE vec2(0.04, 0.10)

#define MOUNTAIN_SCALE 6.0
#define MOUNTAIN_SHIFT 5.3

#define SPEED 11.

const vec3 starColor = vec3(1.0, 1.0, 1.0);

// Color modes
// vec3(#,#,#) Number of bits per channel

// 24 bit color
#define RGB888 vec3(8, 8, 8)
// 16 bit color
#define RGB565 vec3(5, 6, 5)
#define RGB664 vec3(6, 6, 4)
// 8 bit color
#define RGB332 vec3(3, 3, 2)
#define RGB242 vec3(2, 4, 2)
#define RGB222 vec3(2, 2, 2)  //+2 unused

//#define DITHER_ENABLE
#define COLOR_MODE RGB242

// Object IDs
#define SKYDOME 0.
#define FLOOR 1.
#define OCTAGON 3.

float pi = atan(1.0) * 4.0;
float tau = atan(1.0) * 8.0;

float normpdf(in float x, in float sigma) {
  return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

vec3 dither(vec3 color, vec3 bits, vec2 pixel) {
  vec3 cmax = exp2(bits) - 1.0;

  vec3 dithfactor = mod(color, 1.0 / cmax) * cmax;
  float dithlevel = texture(iChannel0, pixel / iChannelResolution[2].xy).r;

  vec3 cl = floor(color * cmax) / cmax;
  vec3 ch = ceil(color * cmax) / cmax;

  return mix(cl, ch, step(dithlevel, dithfactor));
}

struct MC {
  vec3 position;
  vec3 normal;
  float dist;
  float steps;
  float id;
};

// Returns a rotation matrix for the given angles around the X,Y,Z axes.
mat3 Rotate(vec3 angles) {
  vec3 c = cos(angles);
  vec3 s = sin(angles);

  mat3 rotX = mat3(1.0, 0.0, 0.0, 0.0, c.x, s.x, 0.0, -s.x, c.x);
  mat3 rotY = mat3(c.y, 0.0, -s.y, 0.0, 1.0, 0.0, s.y, 0.0, c.y);
  mat3 rotZ = mat3(c.z, s.z, 0.0, -s.z, c.z, 0.0, 0.0, 0.0, 1.0);

  return rotX * rotY * rotZ;
}

//==== Distance field operators/functions by iq. ====
vec2 opU(vec2 d1, vec2 d2) {
  return (d1.x < d2.x) ? d1 : d2;
}

vec2 opS(vec2 d1, vec2 d2) {
  return (-d1.x > d2.x) ? d1 * vec2(-1, 1) : d2;
}

vec2 sdBox(vec3 pos, vec3 size, float id) {
  return vec2(length(max(abs(pos) - size, 0.0)), id);
}

vec2 sdOct(vec3 p, float r, float id) {
  vec2 s = vec2(1, -1) / sqrt(1.0);
  return vec2(
      max(max(max(abs(dot(p, s.xxx)), abs(dot(p, s.yyx))), abs(dot(p, s.yxy))),
          abs(dot(p, s.xyy))) -
          r * mix(1.0, 1.0 / sqrt(3.0), .5),
      id);
}

vec2 sdSphere(vec3 p, float s, float id) {
  return vec2(length(p) - s, id);
}

float displace(vec3 p) {
  float height = 10.;
  return ((cos(1. * p.y + 0.5) * clamp(sin(1.1 * p.x), 0.5, 1.) *
           sin(0. + 2.4) * height *
           clamp(texture(iChannel0, p.xy / 10.).r * 0.4, 0.0, 0.5)));
}

// float displace(vec3 p) {
// return ((cos(1.*p.x)*sin(1.1*p.y)/2.*sin(4.*p.z+1.)))*texture(iChannel2,
// p.xy/10.).r+0.1;
//}

vec2 sdPlane(vec3 p, vec4 n, float id) {
  // n must be normalized
  return vec2(
      dot(vec3(p.x, p.y, max(p.z + displace(vec3(p.x, p.y - 10., p.z)), p.z)),
          vec3(n.x, n.y, n.z)) +
          n.y,
      id);
}

vec2 sdColumn(vec3 p, float r, float id) {
  return vec2(((abs(p.x) + abs(p.y)) - r) / sqrt(2.0), id);
}

// Distance to the scene
vec2 Scene(vec3 p) {
  vec2 d = vec2(MAX_DIST, SKYDOME);
  // d = opU(opU(sdPlane(p, vec4(0, 0,-1, 0), FLOOR), d),
  //        opU(sdPlane(p, vec4(0, 0.5,-1, 0), FLOOR), d)) ;

  // d = opU(sdPlane(p, vec4(0, 0,-1, 0), FLOOR), d);
  // d = opU(sdBox(vec3(p.x-25.1, p.y, p.z-15.4), vec3(5.5, 0.5,15.5), OCTAGON),
  // d); d = opU(sdBox(vec3(p.x-10.1, p.y-2., p.z-10.4), vec3(5.5, 0.5,15.5),
  // OCTAGON), d);
  // d = opU(sdOct(vec3(p.x-30.1, p.y, p.z+0.2*2.), 0.5, OCTAGON), d);

  return d;
}

// Surface normal at the current position
vec3 Normal(vec3 p) {
  vec3 off = vec3(NORMAL_OFFS, 0, 0);
  return normalize(vec3(Scene(p + off.xyz).x - Scene(p - off.xyz).x,
                        Scene(p + off.zxy).x - Scene(p - off.zxy).x,
                        Scene(p + off.yzx).x - Scene(p - off.yzx).x));
}

// Raymarch the scene with the given ray
MC MR(vec3 orig, vec3 dir) {
  float steps = 0.0;
  float dist = 0.0;
  float id = 0.0;

  for (int i = 0; i < MAX_STEPS; i++) {
    vec2 object = Scene(orig + dir * dist);

    // Add the sky dome and have it follow the camera.
    object = opU(object, -sdSphere(dir * dist, MAX_DIST, SKYDOME));

    dist += object.x * STEP_MULT;

    id = object.y;

    steps++;

    if (abs(object.x) < MIN_DIST * dist) {
      break;
    }
  }

  MC result;

  result.position = orig + dir * dist;
  result.normal = Normal(result.position);
  result.dist = dist;
  result.steps = steps;
  result.id = id;

  return result;
}

// Scene texturing/shading
vec3 Shade(MC hit, vec3 direction, vec3 camera) {
  vec3 color = vec3(0.0);
  vec3 rd = color;
  vec3 skydomeColor = color;

  if (hit.id == SKYDOME) {
    // vec2 v = fwidth(hit.position.xy);
    // vec4 soundNoise = texture(iChannel3, v);

    // Sky gradient
    color = mix(SKY_COLOR_1 * 1.4, SKY_COLOR_2, -hit.position.z / 7.0);
    // nice
    // color = mix(SKY_COLOR_1, SKY_COLOR_2, hit.position.z/9.0);

    // Sun
    vec3 sunDir = normalize(SUN_DIRECTION);

    float sun = smoothstep(0.9430, 0.975, dot(direction, sunDir));
    sun -= smoothstep(0.1, 0.9, 0.5);
    vec3 sunCol = mix(SUN_COLOR_1, SUN_COLOR_2 * 1.2, hit.position.z / 2.5);

    color =
        mix(color, sunCol, sun) + texture(iChannel2, vec2(2.) * 0.1).rgb * 0.07;
  }

  // if(hit.id == OCTAGON)
  //{
  //    color = texture(iChannel1, hit.position.xy).rgb *0.5;
  //}

  if (hit.id == FLOOR) {
    vec2 uv = abs(mod(hit.position.xy + GRID_SIZE / 2.0, GRID_SIZE) -
                  GRID_SIZE / 2.0);

    uv /= fwidth(hit.position.xy);

    float gln = min(min(uv.x, uv.y), 1.) / GRID_SIZE;
    color = mix(GRID_COLOR_1,
                GRID_COLOR_2,
                0.7 - smoothstep(0.0, GRID_LINE_SIZE / GRID_SIZE, gln));

    // darker on the sides
    vec3 normal = vec3(0., -0.5, 0.);
    vec3 rfld = reflect(direction, normal);
    float reflectstrength = 1. - abs(dot(direction, normal));
    color *= reflectstrength;
  }

  // Distance fog
  color += mix(GRID_COLOR_2, FOG_COLOR, pow(hit.dist, 1.01)) / 70.;

  return color;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
  vec2 res = iResolution.xy / iResolution.y;
  float aspect = iResolution.x / iResolution.y;
  vec2 uv = fragCoord.xy / iResolution.y;

  // Camera stuff
  vec3 angles = vec3(0);

  // Auto mode
  if (iMouse.xy == vec2(0, 0)) {
    angles.y = tau * (1.8 / 8.0);
    angles.x = tau * (1.8 / 8.0) * 1.1;
  } else {
    angles = vec3((iMouse.xy / iResolution.xy) * pi, 0);
    angles.xy *= vec2(1.0, 1.0);
  }

  angles.y = clamp(angles.y, 0.0, 15.5 * tau / 64.0);

  mat3 rotate = Rotate(angles.yzx);

  vec3 orig = vec3(0, 1., 0.) * rotate;

  vec3 dir = normalize(vec3(uv - res / 2.0, FOCAL_LENGTH)) * rotate;

  orig.z += 0.25;
  // orig.y = sin(iTime)*10.5;
  orig.x += iTime * SPEED;

  // Ray marching
  MC hit = MR(orig, dir);

  // Shading
  vec3 color = Shade(hit, dir, orig);

#ifdef SHOW_RAY_COST
  color = mix(vec3(0, 1, 0), vec3(1, 0, 0), hit.steps / float(MAX_STEPS));
#endif

#ifdef DITHER_ENABLE
  color = dither(color, COLOR_MODE, fragCoord);
#endif

  // particles

  vec2 position = (fragCoord.xy - iResolution.xy * 0.5) / iResolution.x;
  float angle = atan(dir.y, dir.z) / (atan(iTime) - 1. * 1. * PI);
  angle -= floor(angle);
  float rad = length(vec2(dir.x * 0.02, dir.z));

  float angleFract = fract(angle * 10.5);
  float angleRnd = floor(angle * 180.);
  float angleRnd1 = fract(angleRnd * fract(angleRnd * .72035) * 1.1);
  float angleRnd2 = fract(angleRnd * fract(angleRnd * .82657) * 1.724);
  float t = iTime * 20. + angleRnd1 * 1000.;
  float radDist = sqrt(angleRnd2 + float(1));
  float adist = radDist / rad * .1;
  float dist = (t * .2 + adist);
  dist = abs(fract(dist / 20.) - .5);

  // draw only the particles at the top
  if (dir.z < 0.0) {
    color += 0.6 * max(0.0, .7 - dist * 100. / adist) *
             (0.5 - abs(angleFract - .5)) * 1. / adist / radDist;
  } else {
    color += vec3(0.0, 0., 0.2) *
             vec3(max(0.0, .7 - dist * 100. / adist) *
                  (0.5 - abs(angleFract - .5)) * 1. / adist / radDist);
  }

  fragColor = vec4(color, 1.0);
}
