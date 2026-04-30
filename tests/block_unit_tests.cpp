/*
 ____  _            _      _   _       _ _     _____         _
| __ )| | ___   ___| | __ | | | |_ __ (_) |_  |_   _|__  ___| |_
|  _ \| |/ _ \ / __| |/ / | | | | '_ \| | __|   | |/ _ \/ __| __|
| |_) | | (_) | (__|   <  | |_| | | | | | |_    | |  __/\__ \ |_
|____/|_|\___/ \___|_|\_\  \___/|_| |_|_|\__|   |_|\___||___/\__|

Block class unit tests based on tests/TODO.md.
*/

#include "block.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr double k_tol = 1e-4;

double abs_diff(double a, double b)
{
  return std::fabs(a - b);
}

bool approx_equal(double a, double b, double tol = k_tol)
{
  return abs_diff(a, b) <= tol;
}

bool require(bool condition, const std::string &msg)
{
  if (condition) {
    std::cout << "[PASS] " << msg << '\n';
    return true;
  }
  std::cerr << "[FAIL] " << msg << '\n';
  return false;
}

bool require_close(double expected, double got, const std::string &msg)
{
  if (approx_equal(expected, got)) {
    std::cout << "[PASS] " << msg << " expected=" << expected << " got=" << got << '\n';
    return true;
  }
  std::cerr << "[FAIL] " << msg << " expected=" << expected << " got=" << got << '\n';
  return false;
}

bool require_close_at(double expected, double got, double time, const std::string &msg)
{
  if (approx_equal(expected, got)) {
    std::cout << "[PASS] " << msg << " expected=" << expected << " got=" << got
              << " time=" << time << '\n';
    return true;
  }
  std::cerr << "[FAIL] " << msg << " expected=" << expected << " got=" << got
            << " time=" << time << '\n';
  return false;
}

} // namespace

int main()
{
  using cncpp::Block;
  using cncpp::Machine;
  using cncpp::Point;

  int failures = 0;
  Machine machine;

  std::cout << "[RUN] Block tests starting\n";

  std::cout << "[RUN] Unparsed block should throw\n";
  {
    Block block("G1 X1 Y1 Z1 F1000");
    bool threw_desc = false;
    bool threw_lambda = false;
    bool threw_interp_l = false;
    bool threw_interp_t = false;
    bool threw_walk = false;

    try {
      (void)block.desc(false);
    } catch (const std::runtime_error &) {
      threw_desc = true;
    }
    try {
      double speed = 0.0;
      (void)block.lambda(0.0, speed);
    } catch (const std::runtime_error &) {
      threw_lambda = true;
    }
    try {
      (void)block.interpolate(0.5);
    } catch (const std::runtime_error &) {
      threw_interp_l = true;
    }
    try {
      double lambda = 0.0;
      double speed = 0.0;
      (void)block.interpolate(0.0, lambda, speed);
    } catch (const std::runtime_error &) {
      threw_interp_t = true;
    }
    try {
      block.walk([](Block &b, double t, double l, double s) {
        (void)b;
        (void)t;
        (void)l;
        (void)s;
      });
    } catch (const std::runtime_error &) {
      threw_walk = true;
    }

    failures += !require(threw_desc, "desc() throws before parse");
    failures += !require(threw_lambda, "lambda() throws before parse");
    failures += !require(threw_interp_l, "interpolate(lambda) throws before parse");
    failures += !require(threw_interp_t, "interpolate(time) throws before parse");
    failures += !require(threw_walk, "walk() throws before parse");
  }

  std::cout << "[RUN] Line interpolation\n";
  {
    Block start("N1 G00 X0 Y0 Z0");
    start.parse(&machine);

    Block block("N2 G01 X100 Y100 Z0 F1000", start);
    block.parse(&machine);

    failures += !require(block.parsed(), "line: block parsed");
    failures += !require(block.type() == Block::BlockType::LINE, "line: block type LINE");
    failures += !require_close(100.0, block.target().x(), "line: target x");
    failures += !require_close(100.0, block.target().y(), "line: target y");
    failures += !require_close(0.0, block.target().z(), "line: target z");
    failures += !require_close(1000.0, block.feedrate(), "line: feedrate");

    const double dt = block.profile().dt;
    std::cout << "[INFO] line dt=" << dt << '\n';

    double lambda = -1.0;
    double speed = -1.0;
    (void)block.interpolate(0.0, lambda, speed);
    failures += !require_close(0.0, lambda, "line: lambda at t=0");
    failures += !require_close(0.0, speed, "line: speed at t=0");

    (void)block.interpolate(dt, lambda, speed);
    failures += !require_close(1.0, lambda, "line: lambda at t=dt");
    failures += !require_close(0.0, speed, "line: speed at t=dt");

    (void)block.interpolate(dt / 2.0, lambda, speed);
    failures += !require_close(0.5, lambda, "line: lambda at t=dt/2");

    (void)block.interpolate(-0.1, lambda, speed);
    failures += !require_close(0.0, lambda, "line: lambda at t<0");

    (void)block.interpolate(dt + 0.1, lambda, speed);
    failures += !require_close(1.0, lambda, "line: lambda at t>dt");

    const double expected_len = std::hypot(100.0, 100.0);
    failures += !require_close(expected_len, block.length(), "line: length");

    double max_speed = 0.0;
    block.walk([&](Block &b, double t, double l, double s) {
      (void)b;
      (void)t;
      (void)l;
      if (s > max_speed) {
        max_speed = s;
      }
    });
    std::cout << "[INFO] line max speed=" << max_speed << '\n';
    failures += !require(max_speed <= 1000.0, "line: max speed <= 1000");
    failures += !require(max_speed >= 990.0, "line: max speed >= 990");
  }

  std::cout << "[RUN] Arc interpolation (center point, CW)\n";
  {
    Block start("N1 G00 X0 Y0 Z0");
    start.parse(&machine);

    Block block("N2 G02 X100 Y0 Z0 I50 J50 F1000", start);
    block.parse(&machine);

    const double dt = block.profile().dt;
    const double r = block.r();
    const double expected_len = 0.75 * 2.0 * M_PI * r;
    failures += !require_close(expected_len, block.length(), "arc cw ij: length");

    bool y_negative = false;
    bool x_before = false;
    bool x_after = false;
    block.walk([&](Block &b, double t, double l, double s) {
      (void)s;
      Point p = b.interpolate(l);
      if (p.y() < -k_tol) {
        y_negative = true;
        failures += !require_close_at(0.0, p.y(), t, "arc cw ij: Y should be >= 0");
      }
      if (t < dt / 2.0 && p.x() < 50.0 - k_tol) {
        x_before = true;
      }
      if (t > dt / 2.0 && p.x() > 50.0 + k_tol) {
        x_after = true;
      }
    });
    failures += !require(!y_negative, "arc cw ij: Y never negative");
    failures += !require(x_before, "arc cw ij: X < 50 before dt/2");
    failures += !require(x_after, "arc cw ij: X > 50 after dt/2");
  }

  std::cout << "[RUN] Arc interpolation (center point, CCW)\n";
  {
    Block start("N1 G00 X0 Y0 Z0");
    start.parse(&machine);

    Block block("N2 G03 X100 Y0 Z0 I50 J50 F1000", start);
    block.parse(&machine);

    const double dt = block.profile().dt;
    const double r = block.r();
    const double expected_len = 0.25 * 2.0 * M_PI * r;
    failures += !require_close(expected_len, block.length(), "arc ccw ij: length");

    bool y_positive = false;
    bool x_before = false;
    bool x_after = false;
    block.walk([&](Block &b, double t, double l, double s) {
      (void)s;
      Point p = b.interpolate(l);
      if (p.y() > k_tol) {
        y_positive = true;
        failures += !require_close_at(0.0, p.y(), t, "arc ccw ij: Y should be <= 0");
      }
      if (t < dt / 2.0 && p.x() < 50.0 - k_tol) {
        x_before = true;
      }
      if (t > dt / 2.0 && p.x() > 50.0 + k_tol) {
        x_after = true;
      }
    });
    failures += !require(!y_positive, "arc ccw ij: Y never positive");
    failures += !require(x_before, "arc ccw ij: X < 50 before dt/2");
    failures += !require(x_after, "arc ccw ij: X > 50 after dt/2");
  }

  std::cout << "[RUN] Arc interpolation (radius, CW)\n";
  {
    Block start("N1 G00 X0 Y0 Z0");
    start.parse(&machine);

    Block block("N2 G02 X100 Y0 Z0 R60 F1000", start);
    block.parse(&machine);

    const double r = block.r();
    const double half_perimeter = 0.5 * 2.0 * M_PI * r;
    failures += !require(block.length() < half_perimeter,
                         "arc cw r: length less than half perimeter");

    bool y_negative = false;
    bool y_over = false;
    bool x_before = false;
    bool x_after = false;
    const double dt = block.profile().dt;
    block.walk([&](Block &b, double t, double l, double s) {
      (void)s;
      Point p = b.interpolate(l);
      if (p.y() < -k_tol) {
        y_negative = true;
        failures += !require_close_at(0.0, p.y(), t, "arc cw r: Y should be >= 0");
      }
      if (p.y() > 30.0 + k_tol) {
        y_over = true;
        failures += !require_close_at(30.0, p.y(), t, "arc cw r: Y should be <= 30");
      }
      if (t < dt / 2.0 && p.x() < 50.0 - k_tol) {
        x_before = true;
      }
      if (t > dt / 2.0 && p.x() > 50.0 + k_tol) {
        x_after = true;
      }
    });
    failures += !require(!y_negative, "arc cw r: Y never negative");
    failures += !require(!y_over, "arc cw r: Y never above 30");
    failures += !require(x_before, "arc cw r: X < 50 before dt/2");
    failures += !require(x_after, "arc cw r: X > 50 after dt/2");
  }

  std::cout << "[RUN] Arc interpolation (radius, CCW)\n";
  {
    Block start("N1 G00 X0 Y0 Z0");
    start.parse(&machine);

    Block block("N2 G03 X100 Y0 Z0 R-60 F1000", start);
    block.parse(&machine);

    const double r = block.r();
    const double half_perimeter = 0.5 * 2.0 * M_PI * r;
    failures += !require(block.length() > half_perimeter,
                         "arc ccw r: length greater than half perimeter");

    bool y_positive = false;
    bool has_y_above_minus_30 = false;
    bool x_before = false;
    bool x_after = false;
    const double dt = block.profile().dt;
    block.walk([&](Block &b, double t, double l, double s) {
      (void)s;
      Point p = b.interpolate(l);
      if (p.y() > k_tol) {
        y_positive = true;
        failures += !require_close_at(0.0, p.y(), t, "arc ccw r: Y should be <= 0");
      }
      if (p.y() > -30.0 + k_tol) {
        has_y_above_minus_30 = true;
      }
      if (t < dt / 2.0 && p.x() < 50.0 - k_tol) {
        x_before = true;
      }
      if (t > dt / 2.0 && p.x() > 50.0 + k_tol) {
        x_after = true;
      }
    });
    failures += !require(!y_positive, "arc ccw r: Y never positive");
    failures += !require(has_y_above_minus_30,
                         "arc ccw r: some Y values greater than -30");
    failures += !require(x_before, "arc ccw r: X < 50 before dt/2");
    failures += !require(x_after, "arc ccw r: X > 50 after dt/2");
  }

  if (failures == 0) {
    std::cout << "All Block tests passed.\n";
  } else {
    std::cout << failures << " Block test(s) failed.\n";
  }

  return failures;
}
