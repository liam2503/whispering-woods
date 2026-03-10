#include "VectorPP.h"
#include <cmath>

VectorPP::VectorPP() : x(0), y(0) {}

VectorPP::VectorPP(float a_x, float a_y) : x(a_x), y(a_y)
{
}

bool VectorPP::operator==(const VectorPP &a_rVec) const
{
  return (x == a_rVec.x && y == a_rVec.y);
}

bool VectorPP::operator==(const float a_fVal) const
{
  return (x == a_fVal && y == a_fVal);
}

bool VectorPP::operator!=(const VectorPP &a_rVec) const
{
  return (x != a_rVec.x && y != a_rVec.y);
}

VectorPP VectorPP::operator+(const VectorPP &a_rVec) const
{
  return VectorPP(x + a_rVec.x, y + a_rVec.y);
}

VectorPP VectorPP::operator-(const VectorPP &a_rVec) const
{
  return VectorPP(x - a_rVec.x, y - a_rVec.y);
}

VectorPP VectorPP::operator/(const float a_fVal) const
{
  return VectorPP(x / a_fVal, y / a_fVal);
}

VectorPP VectorPP::operator*(const float a_fVal) const
{
  return VectorPP(x * a_fVal, y * a_fVal);
}

void VectorPP::operator+=(const VectorPP &a_rVec)
{
  x += a_rVec.x;
  y += a_rVec.y;
}

void VectorPP::operator-=(const VectorPP &a_rVec)
{
  x -= a_rVec.x;
  y -= a_rVec.y;
}

void VectorPP::operator*=(const float a_fVal)
{
  x *= a_fVal;
  y *= a_fVal;
}

void VectorPP::operator/=(const float a_fVal)
{
  x /= a_fVal;
  y /= a_fVal;
}

bool VectorPP::operator>(const float a_fVal) const
{
  return ((x > a_fVal) && (y > a_fVal));
}

float VectorPP::dist(const VectorPP &a_rVec) const
{
  return sqrt(pow((x - a_rVec.x), 2) + pow(y - a_rVec.y, 2));
}
