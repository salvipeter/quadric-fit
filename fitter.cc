// Code generated by Claude (with some bugfixes) [except QuadricFit::fit()]

// The template magic below computes the exact integral of x^m y^n z^p over a triangle.
// Its equation is given as:
//
//   2A * sum multi(m,{i,j,k}) * multi(n,{l,s,t}) * multi(p,{a,b,c}) *
//     x1^i x2^j x3^k * y1^l y2^s y3^t * z1^a z2^b z3^c * (i+l+a)! (j+s+b)! (k+t+c)! / (m+n+p)!
//
// Here A is the triangle area, multi() is the multinomial coefficient function,
// and {i,j,k}, {l,s,t} and {a,b,c} range over all combinations of m, n, and p, respectively.

// ... but a simple 4-point average (3 vertices and mass center) is good enough.

#include <cmath>
#include <stdexcept>

#include <Eigen/Core>

#include "quadric-fit.hh"

using namespace Eigen;
using namespace Geometry;

namespace QuadricFitSolver {
  VectorXd solve(const MatrixXd &M, const MatrixXd &N, double tolerance);
}

namespace {

// Compile-time factorial
template <int N>
struct Factorial {
  static constexpr long long value = N * Factorial<N-1>::value;
};

template <>
struct Factorial<0> {
  static constexpr long long value = 1;
};

// Compile-time binomial coefficient
template <int N, int K>
struct Binomial {
  static constexpr long long value = 
    (K > N) ? 0 : 
    // (K * 2 > N) ? Binomial<N, N-K>::value :        // <- looks OK, but GCC does not like it
    N * Binomial<N-1, K-1>::value / K;
};

template <int N>
struct Binomial<N, 0> {
  static constexpr long long value = 1;
};

// Forward declaration for compile-time term calculation
template <int I, int J, int K, int L, int S, int T, int A, int B, int C, int M, int N, int P>
struct TermCalculator;

// Recursive template for generating all combinations of a, b, c
template <int I, int J, int K, int L, int S, int T, int A, int B, int M, int N, int P>
struct CLoop {
  template <typename CoordType>
  static double compute(const CoordType& q) {
    constexpr int C = P - A - B;
    double current = TermCalculator<I, J, K, L, S, T, A, B, C, M, N, P>::compute(q);
    double next = CLoop<I, J, K, L, S, T, A, B+1, M, N, P>::compute(q);
    return current + next;
  }
};

// Base case for c loop
template <int I, int J, int K, int L, int S, int T, int A, int M, int N, int P>
struct CLoop<I, J, K, L, S, T, A, P-A+1, M, N, P> {
  template <typename CoordType>
  static double compute(const CoordType&) {
    return 0.0;
  }
};

// Recursive template for generating all combinations of a, b
template <int I, int J, int K, int L, int S, int T, int A, int M, int N, int P>
struct BLoop {
  template <typename CoordType>
  static double compute(const CoordType& q) {
    double current = CLoop<I, J, K, L, S, T, A, 0, M, N, P>::compute(q);
    double next = BLoop<I, J, K, L, S, T, A+1, M, N, P>::compute(q);
    return current + next;
  }
};

// Base case for a loop
template <int I, int J, int K, int L, int S, int T, int M, int N, int P>
struct BLoop<I, J, K, L, S, T, P+1, M, N, P> {
  template <typename CoordType>
  static double compute(const CoordType&) {
    return 0.0;
  }
};

// Recursive template for generating all combinations of l, s, t
template <int I, int J, int K, int L, int S, int M, int N, int P>
struct TLoop {
  template <typename CoordType>
  static double compute(const CoordType& q) {
    constexpr int T = N - L - S;
    double current = BLoop<I, J, K, L, S, T, 0, M, N, P>::compute(q);
    double next = TLoop<I, J, K, L, S+1, M, N, P>::compute(q);
    return current + next;
  }
};

// Base case for s loop
template <int I, int J, int K, int L, int M, int N, int P>
struct TLoop<I, J, K, L, N-L+1, M, N, P> {
  template <typename CoordType>
  static double compute(const CoordType&) {
    return 0.0;
  }
};

// Recursive template for generating all combinations of l
template <int I, int J, int K, int L, int M, int N, int P>
struct LLoop {
  template <typename CoordType>
  static double compute(const CoordType& q) {
    double current = TLoop<I, J, K, L, 0, M, N, P>::compute(q);
    double next = LLoop<I, J, K, L+1, M, N, P>::compute(q);
    return current + next;
  }
};

// Base case for l loop
template <int I, int J, int K, int M, int N, int P>
struct LLoop<I, J, K, N+1, M, N, P> {
  template <typename CoordType>
  static double compute(const CoordType&) {
    return 0.0;
  }
};

// Recursive template for generating all combinations of k
template <int I, int J, int M, int N, int P>
struct KLoop {
  template <typename CoordType>
  static double compute(const CoordType& q) {
    constexpr int K = M - I - J;
    // Only compute if K is non-negative (valid combination)
    double result = (K >= 0) ? LLoop<I, J, K, 0, M, N, P>::compute(q) : 0.0;
    double next = KLoop<I, J+1, M, N, P>::compute(q);
    return result + next;
  }
};

// Base case for j loop
template <int I, int M, int N, int P>
struct KLoop<I, M-I+1, M, N, P> {
  template <typename CoordType>
  static double compute(const CoordType&) {
    return 0.0;
  }
};

// Recursive template for generating all combinations of i
template <int I, int M, int N, int P>
struct ILoop {
  template <typename CoordType>
  static double compute(const CoordType& q) {
    double current = KLoop<I, 0, M, N, P>::compute(q);
    double next = ILoop<I+1, M, N, P>::compute(q);
    return current + next;
  }
};

// Base case for i loop
template <int M, int N, int P>
struct ILoop<M+1, M, N, P> {
  template <typename CoordType>
  static double compute(const CoordType&) {
    return 0.0;
  }
};

// Term calculator for a specific combination of i, j, k, l, s, t, a, b, c
template <int I, int J, int K, int L, int S, int T, int A, int B, int C, int M, int N, int P>
struct TermCalculator {
  template <typename CoordType>
  static double compute(const CoordType& q) {
    constexpr long long totalPower = M + N + P;
      
    // Check if this is a valid combination
    if constexpr (K < 0 || T < 0 || C < 0) {
      return 0.0;
    }
      
    constexpr long long coef = 
      Binomial<M, I>::value * Binomial<M-I, J>::value *
      Binomial<N, L>::value * Binomial<N-L, S>::value *
      Binomial<P, A>::value * Binomial<P-A, B>::value;
      
    constexpr long long num = Factorial<I+L+A>::value * Factorial<J+S+B>::value * Factorial<K+T+C>::value;
    constexpr long long denom = Factorial<totalPower+2>::value;
      
    const double term_coef = static_cast<double>(coef * num) / static_cast<double>(denom);
      
    double term = term_coef;
    term *= std::pow(q[0][0], I) * std::pow(q[1][0], J) * std::pow(q[2][0], K);
    term *= std::pow(q[0][1], L) * std::pow(q[1][1], S) * std::pow(q[2][1], T);
    term *= std::pow(q[0][2], A) * std::pow(q[1][2], B) * std::pow(q[2][2], C);
      
    return term;
  }
};

// Calculate triangle area
template <typename CoordType>
double triangleArea(const CoordType& q) {
  // Cross product of two edges
  double v1x = q[1][0] - q[0][0];
  double v1y = q[1][1] - q[0][1];
  double v1z = q[1][2] - q[0][2];
  
  double v2x = q[2][0] - q[0][0];
  double v2y = q[2][1] - q[0][1];
  double v2z = q[2][2] - q[0][2];
  
  // Cross product components
  double nx = v1y * v2z - v1z * v2y;
  double ny = v1z * v2x - v1x * v2z;
  double nz = v1x * v2y - v1y * v2x;
  
  // Magnitude of the cross product
  double area = 0.5 * sqrt(nx*nx + ny*ny + nz*nz);
  return area;
}

// Main template for triangle integral
template <int M, int N, int P>
struct TriangleIntegral {
  template <typename CoordType>
  static double compute(const CoordType& q) {
    double area = triangleArea(q);
    double integral = ILoop<0, M, N, P>::compute(q);
    return 2.0 * area * integral;
  }
};

// Helper function to make the interface cleaner
// template <int M, int N, int P, typename CoordType>
// double computeTriangleIntegral(const CoordType& q) {
//   return TriangleIntegral<M, N, P>::compute(q);
// }

// Simple alternative

template <int M, int N, int P>
double computeTriangleIntegral(const std::array<Point3D, 3> &q) {
  auto f = [](const Point3D &p) {
    return std::pow(p[0], M) * std::pow(p[1], N) * std::pow(p[2], P);
  };
  return triangleArea(q) * (f(q[0]) + f(q[1]) + f(q[2]) + f((q[0] + q[1] + q[2]) / 3)) / 4;
}

}

void Quadric::fit(const TriMesh &mesh) {
  MatrixXd M = MatrixXd::Zero(10, 10);
  MatrixXd N = MatrixXd::Zero(10, 10);
  std::array<Point3D, 3> triangle;
  double A = 0;
  for (const auto &tri : mesh.triangles()) {
    for (size_t i = 0; i < 3; ++i)
      triangle[i] = mesh[tri[i]];
    A += triangleArea(triangle);
    
    M(0, 0) += computeTriangleIntegral<0,0,0>(triangle);
    M(1, 0) += computeTriangleIntegral<1,0,0>(triangle);
    M(1, 1) += computeTriangleIntegral<2,0,0>(triangle);
    M(2, 0) += computeTriangleIntegral<0,1,0>(triangle);
    M(2, 1) += computeTriangleIntegral<1,1,0>(triangle);
    M(2, 2) += computeTriangleIntegral<0,2,0>(triangle);
    M(3, 0) += computeTriangleIntegral<0,0,1>(triangle);
    M(3, 1) += computeTriangleIntegral<1,0,1>(triangle);
    M(3, 2) += computeTriangleIntegral<0,1,1>(triangle);
    M(3, 3) += computeTriangleIntegral<0,0,2>(triangle);
    M(4, 0) += computeTriangleIntegral<2,0,0>(triangle);
    M(4, 1) += computeTriangleIntegral<3,0,0>(triangle);
    M(4, 2) += computeTriangleIntegral<2,1,0>(triangle);
    M(4, 3) += computeTriangleIntegral<2,0,1>(triangle);
    M(4, 4) += computeTriangleIntegral<4,0,0>(triangle);
    M(5, 0) += computeTriangleIntegral<1,1,0>(triangle);
    M(5, 1) += computeTriangleIntegral<2,1,0>(triangle);
    M(5, 2) += computeTriangleIntegral<1,2,0>(triangle);
    M(5, 3) += computeTriangleIntegral<1,1,1>(triangle);
    M(5, 4) += computeTriangleIntegral<3,1,0>(triangle);
    M(5, 5) += computeTriangleIntegral<2,2,0>(triangle);
    M(6, 0) += computeTriangleIntegral<1,0,1>(triangle);
    M(6, 1) += computeTriangleIntegral<2,0,1>(triangle);
    M(6, 2) += computeTriangleIntegral<1,1,1>(triangle);
    M(6, 3) += computeTriangleIntegral<1,0,2>(triangle);
    M(6, 4) += computeTriangleIntegral<3,0,1>(triangle);
    M(6, 5) += computeTriangleIntegral<2,1,1>(triangle);
    M(6, 6) += computeTriangleIntegral<2,0,2>(triangle);
    M(7, 0) += computeTriangleIntegral<0,2,0>(triangle);
    M(7, 1) += computeTriangleIntegral<1,2,0>(triangle);
    M(7, 2) += computeTriangleIntegral<0,3,0>(triangle);
    M(7, 3) += computeTriangleIntegral<0,2,1>(triangle);
    M(7, 4) += computeTriangleIntegral<2,2,0>(triangle);
    M(7, 5) += computeTriangleIntegral<1,3,0>(triangle);
    M(7, 6) += computeTriangleIntegral<1,2,1>(triangle);
    M(7, 7) += computeTriangleIntegral<0,4,0>(triangle);
    M(8, 0) += computeTriangleIntegral<0,1,1>(triangle);
    M(8, 1) += computeTriangleIntegral<1,1,1>(triangle);
    M(8, 2) += computeTriangleIntegral<0,2,1>(triangle);
    M(8, 3) += computeTriangleIntegral<0,1,2>(triangle);
    M(8, 4) += computeTriangleIntegral<2,1,1>(triangle);
    M(8, 5) += computeTriangleIntegral<1,2,1>(triangle);
    M(8, 6) += computeTriangleIntegral<1,1,2>(triangle);
    M(8, 7) += computeTriangleIntegral<0,3,1>(triangle);
    M(8, 8) += computeTriangleIntegral<0,2,2>(triangle);
    M(9, 0) += computeTriangleIntegral<0,0,2>(triangle);
    M(9, 1) += computeTriangleIntegral<1,0,2>(triangle);
    M(9, 2) += computeTriangleIntegral<0,1,2>(triangle);
    M(9, 3) += computeTriangleIntegral<0,0,3>(triangle);
    M(9, 4) += computeTriangleIntegral<2,0,2>(triangle);
    M(9, 5) += computeTriangleIntegral<1,1,2>(triangle);
    M(9, 6) += computeTriangleIntegral<1,0,3>(triangle);
    M(9, 7) += computeTriangleIntegral<0,2,2>(triangle);
    M(9, 8) += computeTriangleIntegral<0,1,3>(triangle);
    M(9, 9) += computeTriangleIntegral<0,0,4>(triangle);

    N(1, 1) += computeTriangleIntegral<0,0,0>(triangle);
    N(2, 2) += computeTriangleIntegral<0,0,0>(triangle);
    N(3, 3) += computeTriangleIntegral<0,0,0>(triangle);
    N(4, 1) += computeTriangleIntegral<1,0,0>(triangle) * 2;
    N(4, 4) += computeTriangleIntegral<2,0,0>(triangle) * 4;
    N(5, 1) += computeTriangleIntegral<0,1,0>(triangle);
    N(5, 2) += computeTriangleIntegral<1,0,0>(triangle);
    N(5, 4) += computeTriangleIntegral<1,1,0>(triangle) * 2;
    N(5, 5) += computeTriangleIntegral<2,0,0>(triangle);
    N(5, 5) += computeTriangleIntegral<0,2,0>(triangle);
    N(6, 1) += computeTriangleIntegral<0,0,1>(triangle);
    N(6, 3) += computeTriangleIntegral<1,0,0>(triangle);
    N(6, 4) += computeTriangleIntegral<1,0,1>(triangle) * 2;
    N(6, 5) += computeTriangleIntegral<0,1,1>(triangle);
    N(6, 6) += computeTriangleIntegral<2,0,0>(triangle);
    N(6, 6) += computeTriangleIntegral<0,0,2>(triangle);
    N(7, 2) += computeTriangleIntegral<0,1,0>(triangle) * 2;
    N(7, 5) += computeTriangleIntegral<1,1,0>(triangle) * 2;
    N(7, 7) += computeTriangleIntegral<0,2,0>(triangle) * 4;
    N(8, 2) += computeTriangleIntegral<0,0,1>(triangle);
    N(8, 3) += computeTriangleIntegral<0,1,0>(triangle);
    N(8, 5) += computeTriangleIntegral<1,0,1>(triangle);
    N(8, 6) += computeTriangleIntegral<1,1,0>(triangle);
    N(8, 7) += computeTriangleIntegral<0,1,1>(triangle) * 2;
    N(8, 8) += computeTriangleIntegral<0,2,0>(triangle);
    N(8, 8) += computeTriangleIntegral<0,0,2>(triangle);
    N(9, 3) += computeTriangleIntegral<0,0,1>(triangle) * 2;
    N(9, 6) += computeTriangleIntegral<1,0,1>(triangle) * 2;
    N(9, 8) += computeTriangleIntegral<0,1,1>(triangle) * 2;
    N(9, 9) += computeTriangleIntegral<0,0,2>(triangle) * 4;
  }

  M /= A;
  N /= A;
  M = M.selfadjointView<Lower>();
  N = N.selfadjointView<Lower>();

  auto s = QuadricFitSolver::solve(M, N, 1e-8);
  std::copy(s.begin(), s.end(), coeffs.begin());
}
