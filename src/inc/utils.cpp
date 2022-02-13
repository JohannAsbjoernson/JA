#include "rack.hpp"
#include "patch.hpp"
#include "asset.hpp"
#include "utils.hpp"
//#include "plugin.hpp"

int grid_pick(Vec pos, Vec size, Vec res)
{
  int x = floor(rescale(pos.x, 0.0f, size.x, 0.0f, 1.0f) * res.x);
  int y = floor(rescale(pos.y, 0.0f, size.y, 0.0f, 1.0f) * res.y);
  return (y * (int)res.x) + x;
}

Vec equaldist(Vec s, Vec e, int i, int c)
{
  assert(c > 1.f);
  float t = i / (c - 1.f);
  return s.crossfade(e, t);
}

double circle_dist(double a, double b)
{
  double d = abs(a - b);
  return (d > 0.5 ? 0.5 - (d - 0.5) : d) * 2.0;
}

float float_wrap(float x)
{
  if (x >= 0)
    return fmod(x, 1.0f);
  else
    return 1.0f - fmod(abs(x), 1.0f);
}
float cvToBpm(float x)
{
  return 1.0f / std::pow(2.0f, x);
}
double lerpd(double a, double b, double t)
{
  t = clamp(t, 0.0, 1.0);
  return a * (1.0 - t) + b * t;
}
float lerpf(float a, float b, float t)
{
  return a * (1 - t) + b * t;
}
double ilerpd(double a, double b, double p)
{
  return (p - a) / (b - a);
}
float ilerpf(float a, float b, float p)
{
  return (p - a) / (b - a);
}
Vec lerpv(float ax, float ay, float bx, float by, float t)
{
  return Vec(lerpf(ax, bx, t), lerpf(ay, by, t));
}
Vec lerpv(Vec a, Vec b, float t)
{
  return a.crossfade(b, t);
}

double clamp(double x, double a, double b)
{
  return x < a ? a : x > b ? b
                           : x;
}
float clampf(float x, float a, float b)
{
  return x < a ? a : x > b ? b
                           : x;
}
// float clamp(float x, float a, float b){
//   return x < a ? a : x > b ? b : x;
// }
double sign(double x)
{
  return x < 0 ? -1.0 : 1.0;
}
int sign(float x)
{
  return x > 0 ? 1 : (x < 0 ? -1 : 0);
}
int sign(int x)
{
  return x > 0 ? 1 : (x < 0 ? -1 : 0);
}
// template <typename T>
// T max(T a, T b){
//   return a < b ? a : b;

// }
// template <typename T>
// T min(T a, T b){
//   return a > b ? a : b;
// }

double randomBi()
{
  return rack::random::uniform() * 2.0 - 1.0;
}
double cvToHertz(double cv)
{
  return 261.626 * powf(2.0, cv);
}
int wrap(int i, int s)
{
  if (i < 0)
    return s - (abs(i) % s);
  else
    return i % s;
}
bool inside(double x, double s, double e)
{
  return (x >= s && x < e);
}
bool inside(int w, int h, Vec p)
{
  return p.x < w && p.x >= 0 && p.y < h && p.y >= 0;
}

double mixin(double a, double b)
{
  a *= 0.2;
  b *= 0.2;
  return (a + b - (a * b)) * 5.0;
}
double sigmoid(double x)
{
  const double E = 2.71828;
  const double ET = pow(E, x);
  return ET / (ET + 1.0);
}
float sigmoid(float x)
{
  const float E = 2.71828f;
  const float ET = pow(E, x);
  return ET / (ET + 1.0);
}
float sdist(float x, float y, float _x, float _y)
{
  return pow(abs(x - _x), 2) + pow(abs(y - _y), 2);
}

void print_vec(Vec v, std::string s)
{
  printf("%s : ", s.c_str());
  printf("( %f, ", v.x);
  printf("%f )\n", v.y);
}

void print_rect(Rect r, std::string s)
{
  printf("%s : \n", s.c_str());
  print_vec(r.pos, "  pos ");
  print_vec(r.size, "  size");
}
void PatchPath::onAction(const event::Action &e)
{
  APP->patch->load(path);
}
