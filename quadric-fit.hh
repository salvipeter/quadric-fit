#pragma once

#include <geometry.hh>          // https://github.com/salvipeter/libgeom/

// Based on:
//   D. Yan, Y. Liu, W. Wang:
//     Quadric surface extraction by variational shape approximation.
//       Proceedings of GMP'06, LNCS 4077, pp. 73-86, 2006.
//   (Sections 2.2-3)

struct QuadricFit {
  std::array<double, 10> coeffs; // 1 x y z x^2 xy xz y^2 yz z^2
  double eval(const Geometry::Point3D &p) const;
  Geometry::Vector3D grad(const Geometry::Point3D &p) const;
  void fit(const Geometry::TriMesh &mesh);
  double distance(const Geometry::Point3D &p) const;
};
