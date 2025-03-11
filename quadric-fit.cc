#include "quadric-fit.hh"

using namespace Geometry;

double QuadricFit::eval(const Point3D &p) const {
  return
    coeffs[0] +
    coeffs[1] * p[0] + coeffs[2] * p[1] + coeffs[3] * p[2] +
    coeffs[4] * p[0] * p[0] + coeffs[5] * p[0] * p[1] + coeffs[6] * p[0] * p[2] +
    coeffs[7] * p[1] * p[1] + coeffs[8] * p[1] * p[2] + coeffs[9] * p[2] * p[2];
}

Vector3D QuadricFit::grad(const Point3D &p) const {
  return {
    coeffs[1] + coeffs[4] * 2 * p[0] + coeffs[5] * p[1] + coeffs[6] * p[2],
    coeffs[2] + coeffs[5] * p[0] + coeffs[7] * 2 * p[1] + coeffs[8] * p[2],
    coeffs[3] + coeffs[6] * p[0] + coeffs[8] * p[1] + coeffs[9] * 2 * p[2]
  };
}

double QuadricFit::distance(const Point3D &p) const {
  return 42;                    // TODO
}
