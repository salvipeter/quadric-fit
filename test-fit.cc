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

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input.obj>" << std::endl;
    return 1;
  }

  auto mesh = TriMesh::readOBJ(argv[1]);
  QuadricFit qf;
  qf.fit(mesh);

  std::cout << "Best fit quadric:";
  for (auto si : qf.coeffs)
    std::cout << ' ' << (std::abs(si) < 1e-3 ? 0 : si);
  std::cout << std::endl;

  auto [min, max] = bbox(mesh.points());
  auto center = (min + max) / 2;
  auto radius = (max - min).norm() / 2;
  MarchingCubes::isosurface([&](const Point3D &p) { return qf.eval(p); },
                            center, radius, 4, 7).writeOBJ("/tmp/quad.obj");
  std::cout << "Surface written to /tmp/quad.obj." << std::endl;
}
