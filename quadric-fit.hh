#pragma once

#include <geometry.hh>          // https://github.com/salvipeter/libgeom/

// Based on:
//   D. Yan, Y. Liu, W. Wang:
//     Quadric surface extraction by variational shape approximation.
//       Proceedings of GMP'06, LNCS 4077, pp. 73-86, 2006.
//   (Sections 2.2-3, and references within, notably [18] and [20], also
//    G. Taubin: Distance approximations for rasterizing implicit curves,
//      ACM TOG 13(1), pp. 3-42, 1994.)

struct Quadric {
  // Coefficients corresponding to: 1   x   y   z  x^2  xy  xz y^2  yz  z^2
  std::array<double, 10> coeffs; // c0  c1  c2  c3  c4  c5  c6  c7  c8  c9

  // (Q, P, R) s.t. [x y z] Q [x,y,z] + [x y z] P + R = 0
  std::tuple<Geometry::Matrix3x3, Geometry::Vector3D, double> matrixForm() const;

  // Evaluators
  double eval(const Geometry::Point3D &p) const;
  Geometry::Vector3D grad(const Geometry::Point3D &p) const;

  // Approximation of the Euclidean distance (Taubin's second-order formula)
  double distance(const Geometry::Point3D &p) const;

  // Fitter (eigenvalues <= tolerance are treated as zero)
  void fit(const Geometry::TriMesh &mesh, double tolerance = 1e-8);

  // Classification
  enum Type {
    ELLIPSOID, ELLIPTIC_PARABOLOID, HYPERBOLIC_PARABOLOID,
    HYPERBOLOID_1SHEET, HYPERBOLOID_2SHEETS,
    ELLIPTIC_CONE, ELLIPTIC_CYLINDER, HYPERBOLIC_CYLINDER, PARABOLIC_CYLINDER,
    TWO_PLANES, NO_SURFACE
  };
  Type classify() const;
};
