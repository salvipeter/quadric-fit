#include <marching.hh>          // https://github.com/salvipeter/marching/

#include "quadric-fit.hh"

using namespace Geometry;

auto bbox(const PointVector &pv) {
  Point3D min(pv[0]), max(pv[0]);
  for (const auto &p : pv)
    for (size_t i = 0; i < 3; ++i)
      if (p[i] < min[i])
        min[i] = p[i];
      else if (p[i] > max[i])
        max[i] = p[i];
  return std::make_pair(min, max);
}

std::string names[] = {
  "no surface", "plane", "product of two planes",
  "ellipsoid", "elliptic paraboloid", "hyperbolic paraboloid",
  "hyperboloid of 1 sheet", "hyperboloid of 2 sheets",
  "elliptic cone", "elliptic cylinder", "hyperbolic cylinder", "parabolic cylinder"
};

// https://mathworld.wolfram.com/QuadraticSurface.html
std::array<double, 10> canonical_coeffs[] = {
  //   1   x   y   z   x2  xy  xz  y2  yz  z2
  { {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  } }, //  <placeholder>
  { {8,7,6,5,-5,4,3,4,2,3}},
  { {  0,  0,  0,  0,  1,  0,  0,  0,  0,  0  } }, //  1. coincident planes
  { {  1,  0,  0,  0,  1,  0,  0,  1,  0,  1  } }, //  2. ellipsoid (imaginary)
  { { -1,  0,  0,  0,  1,  0,  0,  1,  0,  1  } }, //  3. ellipsoid (real)
  { {  0,  0,  0,  0,  1,  0,  0,  1,  0,  1  } }, //  4. elliptic cone (imaginary)
  { {  0,  0,  0,  0,  1,  0,  0,  1,  0, -1  } }, //  5. elliptic cone (real)
  { {  1,  0,  0,  0,  1,  0,  0,  1,  0,  0  } }, //  6. elliptic cylinder (imaginary)
  { { -1,  0,  0,  0,  1,  0,  0,  1,  0,  0  } }, //  7. elliptic cylinder (real)
  { {  0,  0,  0, -1,  1,  0,  0,  1,  0,  0  } }, //  8. elliptic paraboloid
  { {  1,  0,  0,  0,  1,  0,  0, -1,  0,  0  } }, //  9. hyperbolic cylinder
  { {  0,  0,  0,  1,  1,  0,  0, -1,  0,  0  } }, // 10. hyperbolic paraboloid
  { { -1,  0,  0,  0,  1,  0,  0,  1,  0, -1  } }, // 11. hyperboloid of one sheet
  { {  1,  0,  0,  0,  1,  0,  0,  1,  0, -1  } }, // 12. hyperboloid of two sheets
  { {  0,  0,  0,  0,  1,  0,  0,  1,  0,  0  } }, // 13. intersecting planes (imaginary)
  { {  0,  0,  0,  0,  1,  0,  0, -1,  0,  0  } }, // 14. intersecting planes (real)
  { {  0,  0,  0,  1,  1,  0,  0,  0,  0,  0  } }, // 15. parabolic cylinder
  { {  1,  0,  0,  0,  1,  0,  0,  0,  0,  0  } }, // 16. parallel planes (imaginary)
  { { -1,  0,  0,  0,  1,  0,  0,  0,  0,  0  } }  // 17. parallel planes (real)
};

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << std::endl;
    std::cerr << "  " << argv[0] << " <input.obj>" << std::endl;
    std::cerr << "Or:" << std::endl;
    std::cerr << "  " << argv[0] << " <default surface # (1-17)>" << std::endl;
    return 1;
  }

  Point3D center(0, 0, 0);
  double radius = 2;

  Quadric qf;
  int canonical = std::atoi(argv[1]);
  if (canonical <= 0 || canonical > 17) {
    auto mesh = TriMesh::readOBJ(argv[1]);
    qf.fit(mesh);
    auto [min, max] = bbox(mesh.points());
    center = (min + max) / 2;
    radius = (max - min).norm() / 2;
  } else
    qf.coeffs = canonical_coeffs[canonical];

  std::cout << "Quadric:";
  for (auto si : qf.coeffs)
    std::cout << ' ' << si;
  std::cout << std::endl;
  std::cout << "Its type seems to be: " << names[qf.classify()] << std::endl;

  MarchingCubes::isosurface([&](const Point3D &p) { return qf.eval(p); },
                            center, radius, 4, 7).writeOBJ("/tmp/quad.obj");
  std::cout << "Surface written to /tmp/quad.obj." << std::endl;
}
