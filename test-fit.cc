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
  if (false) {
    QuadricFit qf;
    for (size_t i = 0; i < 10; ++i)
      qf.coeffs[i] = 0;
    qf.coeffs[0] = -1;
    qf.coeffs[4] = 1;
    qf.coeffs[7] = 1;
    qf.coeffs[9] = 1;
    Point3D p(0.95,0,0);
    auto d0 = std::abs(qf.eval(p));
    auto d1 = std::abs(qf.eval(p)) / qf.grad(p).norm();
    auto d2 = qf.distance(p);
    std::cout << d0 << " / " << d1 << " / " << d2 << std::endl;
    return 0;
  }

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
