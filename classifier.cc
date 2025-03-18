#include <Eigen/Dense>

#include "quadric-fit.hh"

using namespace Eigen;

Quadric::Type Quadric::classify(double tolerance) const {
  auto [Q, P, R] = matrixForm();
  Map<const Matrix<double, 3, 3, RowMajor>> A(Q.data());
  Map<const Vector3d> B(P.data());
  double C = R;

  SelfAdjointEigenSolver<Matrix3d> eigensolver(A);
  if (eigensolver.info() != Success)
    throw std::runtime_error("Eigenvalue decomposition failed.");
  Vector3d eigenvalues = eigensolver.eigenvalues();
  Matrix3d eigenvectors = eigensolver.eigenvectors();

  auto fuzzy = [=](double x) {
    return std::abs(x) <= tolerance ? 0.0 : x;
  };

  // Count positive, negative, and zero eigenvalues and save non-zero indices
  int positiveCount = 0, negativeCount = 0, zeroCount = 0;
  std::vector<int> indices;
  for (int i = 0; i < 3; ++i) {
    if (fuzzy(eigenvalues[i]) == 0)
      ++zeroCount;
    else {
      indices.push_back(i);
      if (eigenvalues[i] > 0)
        ++positiveCount;
      else
        ++negativeCount;
    }
  }

  // Classify based on eigenvalues and rank

  if (zeroCount == 3)
    return (B[0] != 0 || B[1] != 0 || B[2] != 0) ? PLANE : NO_SURFACE;

  if (zeroCount == 2) {
    Vector3d w2 = eigenvectors.col(indices[0]);
    Vector3d w0;
    if (std::abs(w2[0]) > std::abs(w2[1]))
      w0 << -w2[2], 0, w2[0];
    else
      w0 << 0, w2[2], -w2[1];
    Vector3d w1 = w2.cross(w0);
    auto d0 = fuzzy(w0.dot(B));
    auto d1 = fuzzy(w1.dot(B));
    if (d0 != 0 || d1 != 0)
      return PARABOLIC_CYLINDER;
    auto e2 = w2.dot(A * w2);
    auto d2 = w2.dot(B);
    auto r = fuzzy(d2 * d2 / (4 * e2) - C);
    if (positiveCount == 1)
      return r > 0 ? TWO_PLANES : (r < 0 ? NO_SURFACE : PLANE);
    return r < 0 ? TWO_PLANES : (r > 0 ? NO_SURFACE : PLANE);
  }

  if (zeroCount == 1) {
    Vector3d w1 = eigenvectors.col(indices[0]);
    Vector3d w2 = eigenvectors.col(indices[1]);
    Vector3d w0 = w1.cross(w2);
    w2 = w0.cross(w1);
    auto d0 = fuzzy(w0.dot(B));
    if (d0 != 0)
      return positiveCount == negativeCount ? HYPERBOLIC_PARABOLOID : ELLIPTIC_PARABOLOID;
    Vector3d Aw1 = A * w1, Aw2 = A * w2;
    Matrix2d E; E << w1.dot(Aw1), w1.dot(Aw2), w1.dot(Aw2), w2.dot(Aw2);
    Vector2d f; f << w1.dot(B), w2.dot(B);
    auto r = fuzzy(f.dot(E.inverse() * f) / 4 - C);
    if (positiveCount == 2)
      return r > 0 ? ELLIPTIC_CYLINDER : NO_SURFACE;
    if (negativeCount == 2)
      return r < 0 ? ELLIPTIC_CYLINDER : NO_SURFACE;
    return r != 0 ? HYPERBOLIC_CYLINDER : TWO_PLANES;
  }

  // Full rank
  double r = fuzzy(B.dot(A.inverse() * B) / 4 - C);
  if (positiveCount == 3)
    return r > 0 ? ELLIPSOID : NO_SURFACE;
  if (positiveCount == 2)
    return r > 0 ? HYPERBOLOID_1SHEET : (r < 0 ? HYPERBOLOID_2SHEETS : ELLIPTIC_CONE);
  if (positiveCount == 1)
    return r > 0 ? HYPERBOLOID_2SHEETS : (r < 0 ? HYPERBOLOID_1SHEET : ELLIPTIC_CONE);
  return r < 0 ? ELLIPSOID : NO_SURFACE;
}
