#include "Vector2D.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

bool almost_equal(double a, double b, double eps = 1e-9) {
  return std::fabs(a - b) <= eps;
}

int run_all_tests() {
  int failures = 0;

  auto check = [&failures](bool condition, const std::string& message) {
    if (!condition) {
      ++failures;
      std::cerr << "[FAIL] " << message << '\n';
    }
  };

  // Constructors and accessors
  {
    Vector2D v_default;
    check(almost_equal(v_default.x(), 0.0), "default ctor sets x to 0");
    check(almost_equal(v_default.y(), 0.0), "default ctor sets y to 0");
    check(v_default.label().empty(), "default ctor sets empty label");

    Vector2D v_xy(3.0, 4.0);
    check(almost_equal(v_xy.x(), 3.0), "(x,y) ctor sets x");
    check(almost_equal(v_xy.y(), 4.0), "(x,y) ctor sets y");
    check(v_xy.label().rfind("vec_", 0) == 0, "(x,y) ctor auto-generates label");

    Vector2D v_labeled("pt", 1.25, -2.5);
    check(v_labeled.label() == "pt", "(label,x,y) ctor sets label");
    check(almost_equal(v_labeled.x(), 1.25), "(label,x,y) ctor sets x");
    check(almost_equal(v_labeled.y(), -2.5), "(label,x,y) ctor sets y");
  }

  // Mutators
  {
    Vector2D v;
    v.set_label("new_name");
    v.set(7.5, -3.25);
    check(v.label() == "new_name", "set_label updates label");
    check(almost_equal(v.x(), 7.5), "set updates x");
    check(almost_equal(v.y(), -3.25), "set updates y");
  }

  // Math behavior
  {
    Vector2D a("a", 3.0, 4.0);
    Vector2D b("b", -1.0, 2.0);

    check(almost_equal(a.norm(), 5.0), "norm computes Euclidean norm");
    check(almost_equal(a.distance_to(b), std::hypot(4.0, 2.0)),
          "distance_to computes Euclidean distance");

    Vector2D sum = a + b;
    check(almost_equal(sum.x(), 2.0), "operator+ x");
    check(almost_equal(sum.y(), 6.0), "operator+ y");

    Vector2D diff = a - b;
    check(almost_equal(diff.x(), 4.0), "operator- x");
    check(almost_equal(diff.y(), 2.0), "operator- y");

    Vector2D scaled = a * 2.5;
    check(almost_equal(scaled.x(), 7.5), "operator* x");
    check(almost_equal(scaled.y(), 10.0), "operator* y");
  }

  // CSV and stream output
  {
    Vector2D v("csv_label", 1.5, -2.0);
    check(Vector2D::to_csv_header() == "label,x,y", "to_csv_header output");
    check(v.to_csv_row() == "csv_label,1.5,-2", "to_csv_row output format");

    std::ostringstream oss;
    oss << v;
    check(oss.str() == "[csv_label] (1.5, -2)", "operator<< output format");
  }

  // Smoke test for full class usage, including timing with 100 elements.
  {
    auto t0 = std::chrono::steady_clock::now();

    std::vector<Vector2D> values;
    values.reserve(100);
    for (int i = 0; i < 100; ++i) {
      values.emplace_back("v_" + std::to_string(i), static_cast<double>(i),
                          -0.5 * static_cast<double>(i));
    }

    Vector2D accumulator;
    accumulator.set_label("acc");
    accumulator.set(0.0, 0.0);

    for (const auto& value : values) {
      accumulator = accumulator + value;
      (void)value.norm();
      (void)value.to_csv_row();
    }

    const auto csv_header = Vector2D::to_csv_header();
    const auto csv_row = accumulator.to_csv_row();
    std::ostringstream oss;
    oss << accumulator;

    auto t1 = std::chrono::steady_clock::now();
    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    check(values.size() == 100, "smoke: vector contains 100 instances");
    check(csv_header == "label,x,y", "smoke: csv header generation");
    check(!csv_row.empty(), "smoke: csv row generation");
    check(!oss.str().empty(), "smoke: stream output generation");

    // Wide upper bound to avoid flaky failures on slow debug environments.
    check(elapsed_ms < 1000,
          "smoke: 100-instance vector workflow executes under 1000 ms");

    std::cout << "[INFO] smoke test elapsed time: " << elapsed_ms << " ms\n";
  }

  return failures;
}

}  // namespace

int main() {
  const int failures = run_all_tests();
  if (failures == 0) {
    std::cout << "[PASS] All Vector2D tests passed\n";
    return 0;
  }

  std::cerr << "[FAIL] " << failures << " Vector2D test(s) failed\n";
  return 1;
}
