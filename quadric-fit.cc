#include <cmath>
#include <stdexcept>

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
  // For an explanation of this formula, see Taubin '94, Eq. (5.3):
  //   F_h = { f_ijk / sqrt(multinom(h,{i,j,k})) | i+j+k = h }
  // and for quadratic surfaces the root of
  //   |F_0| - |F_1| d - |F_2| d^2
  // ... is a lower bound for the Euclidean distance (this generalizes to higher order).
  // This is positive at d = 0 and monotonically decreases for d > 0,
  //   so the equation should have a unique positive root.
  // Note that
  //   |F_0| = abs(f)  and  |F_1| = |grad(f)|,
  // also
  //   F_2 = { f_200, f_020, f_002, f_110 / √2, f_101 / √2, f_011 / √2 }.

  // But Yan'06 has the role of `a` and `b` swapped... that should be an error!
  auto a = -std::sqrt(Vector3D(coeffs[5], coeffs[6], coeffs[8]).normSqr() / 2 +
                      Vector3D(coeffs[4], coeffs[7], coeffs[9]).normSqr());
  auto b = -grad(p).norm();
  auto c = std::abs(eval(p));

  auto D = b * b - 4 * a * c;
  return (-b - std::sqrt(D)) / (2 * a);

  // Cautious version
  // if (D < 0)
  //   throw std::runtime_error("no real solution to quadratic equation");
  // D = std::sqrt(D);
  // auto x1 = (-b + D) / (2 * a);
  // auto x2 = (-b - D) / (2 * a);
  // if (x1 < 0 && x2 < 0)
  //   throw std::runtime_error("no non-negative solution to quadratic equation");
  // if (x1 >= 0 && x2 >= 0 && D != 0)
  //   throw std::runtime_error("multiple non-negative solutions to quadratic equation");
  // return std::max(x1, x2);
}
