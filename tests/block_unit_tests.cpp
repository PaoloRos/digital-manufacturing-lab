/*
 ____  _            _      _   _       _ _     _____         _   
| __ )| | ___   ___| | __ | | | |_ __ (_) |_  |_   _|__  ___| |_ 
|  _ \| |/ _ \ / __| |/ / | | | | '_ \| | __|   | |/ _ \/ __| __|
| |_) | | (_) | (__|   <  | |_| | | | | | |_    | |  __/\__ \ |_ 
|____/|_|\___/ \___|_|\_\  \___/|_| |_|_|\__|   |_|\___||___/\__|
                                      

Covers the public API of cncpp::Block and reports method-level failures.
*/

#include "block.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr double kEps = 1e-9;

struct ExpectedProfile {
  double dt_1 = 0.0;
  double dt_m = 0.0;
  double dt_2 = 0.0;
  double dt = 0.0;
  double f = 0.0;
  double a = 0.0;
  double d = 0.0;
};

ExpectedProfile expected_profile(double length, double feedrate, const cncpp::Machine &machine)
{
  ExpectedProfile out;
  const double a_max = machine.A();
  const double tq = machine.tq();
  double dq = 0.0;

  double f_m = feedrate / 60.0;
  double dt_1 = f_m / a_max;
  double dt_2 = dt_1;
  double dt_m = length / f_m - (dt_1 + dt_2) / 2.0;

  auto quantize = [&](double t) {
    double q = static_cast<size_t>(t / tq + 1) * tq;
    dq = q - t;
    return q;
  };

  double dt = 0.0;
  if (dt_m > 0.0) {
    dt = quantize(dt_1 + dt_m + dt_2);
    dt_m += dq;
    f_m = (2.0 * length) / (dt_1 + dt_2 + 2.0 * dt_m);
  } else {
    dt_1 = dt_2 = std::sqrt(length / a_max);
    dt = quantize(dt_1 + dt_2);
    dt_m = 0.0;
    dt_2 += dq;
    f_m = (2.0 * length) / (dt_1 + dt_2);
  }

  out.dt_1 = dt_1;
  out.dt_m = dt_m;
  out.dt_2 = dt_2;
  out.dt = dt;
  out.f = f_m;
  out.a = f_m / dt_1;
  out.d = -(f_m / dt_2);
  return out;
}

bool approx_equal(double a, double b)
{
  return std::fabs(a - b) <= kEps * std::max(1.0, std::max(std::fabs(a), std::fabs(b)));
}

bool require(bool condition, const std::string &msg)
{
  if (!condition) {
    std::cerr << "[FAIL] " << msg << '\n';
    return false;
  }
  return true;
}

} // namespace

int main()
{
  using cncpp::Block;
  using cncpp::Machine;
  using cncpp::Point;

  int failures = 0;
  Machine machine;

  // LIFECYCLE + ACCESSORS (default state)
  {
    Block b("G1 X10 Y20 Z30 F1200 S800 T2 M3");
    failures += !require(b.line() == "G1 X10 Y20 Z30 F1200 S800 T2 M3", "line(): stores input line");
    failures += !require(!b.parsed(), "parsed(): false before parse");
    failures += !require(b.n() == 0U, "n(): default block number");
    failures += !require(b.type() == Block::BlockType::NO_MOTION, "type(): default NO_MOTION");
    failures += !require(b.tool() == 0U, "tool(): default zero");
    failures += !require(approx_equal(b.feedrate(), 0.0), "feedrate(): default zero");
    failures += !require(approx_equal(b.arc_feedrate(), 0.0), "arc_feedrate(): default zero");
    failures += !require(approx_equal(b.spindle(), 0.0), "spindle(): default zero");
    failures += !require(approx_equal(b.length(), 0.0), "length(): default zero");
    failures += !require(b.m() == 0U, "m(): default zero");
    failures += !require(approx_equal(b.r(), 0.0), "r(): default zero");
    failures += !require(approx_equal(b.theta_0(), 0.0), "theta_0(): default zero");
    failures += !require(approx_equal(b.dtheta(), 0.0), "dtheta(): default zero");
    failures += !require(b.prev == nullptr, "prev: default nullptr");
    failures += !require(b.next == nullptr, "next: default nullptr");
  }

  // desc() before parse should throw
  {
    Block b("G1 X1 Y2 Z3");
    bool threw = false;
    try {
      (void)b.desc(false);
    } catch (const std::runtime_error &) {
      threw = true;
    }
    failures += !require(threw, "desc() before parse: throws runtime_error");
  }

  // parse() + desc() + interpolate(lambda) + interpolate(time,...) + lambda(time,...)
  {
    Block b("N10 G1 X10 Y20 Z30 F1200 S800 T2 M3");

    b.parse(&machine);
    failures += !require(b.parsed(), "parse(): parsed() becomes true");
    failures += !require(b.n() == 10U, "parse(): N token");
    failures += !require(b.type() == Block::BlockType::LINE, "parse(): G token");
    failures += !require(approx_equal(b.target().x(), 10.0), "parse(): X token");
    failures += !require(approx_equal(b.target().y(), 20.0), "parse(): Y token");
    failures += !require(approx_equal(b.target().z(), 30.0), "parse(): Z token");
    failures += !require(approx_equal(b.feedrate(), 1200.0), "parse(): F token");
    failures += !require(approx_equal(b.spindle(), 800.0), "parse(): S token");
    failures += !require(b.tool() == 2U, "parse(): T token");
    failures += !require(b.m() == 3U, "parse(): M token");

    const std::string d = b.desc(false);
    failures += !require(!d.empty(), "desc(false): non-empty after parse");

    double speed = -1.0;
    const double l = b.lambda(0.0, speed);
    failures += !require(approx_equal(l, 0.0), "lambda(time,...): returns stub value");
    failures += !require(approx_equal(speed, 0.0), "lambda(time,...): updates output speed");

    const Point p_mid = b.interpolate(0.5);
    failures += !require(approx_equal(p_mid.x(), 5.0), "interpolate(lambda): x at 0.5");
    failures += !require(approx_equal(p_mid.y(), 10.0), "interpolate(lambda): y at 0.5");
    failures += !require(approx_equal(p_mid.z(), 15.0), "interpolate(lambda): z at 0.5");

    double l_out = -1.0;
    double s_out = -1.0;
    const Point p_t = b.interpolate(0.0, l_out, s_out);
    failures += !require(approx_equal(l_out, 0.0), "interpolate(time,...): returns lambda output");
    failures += !require(approx_equal(s_out, 0.0), "interpolate(time,...): returns speed output");
    failures += !require(approx_equal(p_t.x(), 0.0), "interpolate(time,...): x at t=0");
    failures += !require(approx_equal(p_t.y(), 0.0), "interpolate(time,...): y at t=0");
    failures += !require(approx_equal(p_t.z(), 0.0), "interpolate(time,...): z at t=0");
  }

  // compute(): long block -> trapezoidal profile with quantized duration
  {
    Block b("G1 X200 Y0 Z0 F1200");
    b.parse(&machine);

    const auto &profile = b.profile();
    const auto expected = expected_profile(b.length(), b.feedrate(), machine);
    failures += !require(approx_equal(b.length(), 200.0), "compute(long): path length");
    failures += !require(approx_equal(profile.dt_1, expected.dt_1), "compute(long): dt_1");
    failures += !require(approx_equal(profile.dt_2, expected.dt_2), "compute(long): dt_2");
    failures += !require(approx_equal(profile.dt_m, expected.dt_m), "compute(long): dt_m after quantization");
    failures += !require(approx_equal(profile.dt, expected.dt), "compute(long): quantized total duration");
    failures += !require(approx_equal(profile.f, expected.f), "compute(long): adjusted peak feedrate");
    failures += !require(approx_equal(profile.a, expected.a), "compute(long): acceleration");
    failures += !require(approx_equal(profile.d, expected.d), "compute(long): deceleration");

    double speed = -1.0;
    const double l0 = b.lambda(0.0, speed);
    failures += !require(approx_equal(l0, 0.0), "lambda(long): at t=0");
    failures += !require(approx_equal(speed, 0.0), "lambda(long): speed at t=0");

    const double t_acc = profile.dt_1 / 2.0;
    const double l_acc = b.lambda(t_acc, speed);
    failures += !require(l_acc > 0.0 && l_acc < 1.0, "lambda(long): acceleration phase progress");
    failures += !require(approx_equal(speed, profile.a * t_acc * 60.0), "lambda(long): acceleration phase speed");

    const double t_cruise = profile.dt_1 + profile.dt_m / 2.0;
    const double l_cruise = b.lambda(t_cruise, speed);
    failures += !require(l_cruise > l_acc, "lambda(long): cruise phase progress increases");
    failures += !require(approx_equal(speed, profile.f * 60.0), "lambda(long): cruise speed");

    const double l_end = b.lambda(profile.dt, speed);
    failures += !require(approx_equal(l_end, 1.0), "lambda(long): at end of block");
    failures += !require(approx_equal(speed, 0.0), "lambda(long): speed at end of block");
  }

  // compute(): short block -> triangular profile
  {
    Block b("G1 X0.1 Y0 Z0 F1200");
    b.parse(&machine);

    const auto &profile = b.profile();
    const auto expected = expected_profile(b.length(), b.feedrate(), machine);
    failures += !require(approx_equal(b.length(), 0.1), "compute(short): path length");
    failures += !require(approx_equal(profile.dt_1, expected.dt_1), "compute(short): dt_1");
    failures += !require(approx_equal(profile.dt_2, expected.dt_2), "compute(short): dt_2 after quantization");
    failures += !require(approx_equal(profile.dt_m, expected.dt_m), "compute(short): no cruise phase");
    failures += !require(approx_equal(profile.dt, expected.dt), "compute(short): quantized total duration");
    failures += !require(approx_equal(profile.f, expected.f), "compute(short): peak feedrate");

    double speed = -1.0;
    const double t_mid = profile.dt_1 / 2.0;
    const double l_mid = b.lambda(t_mid, speed);
    failures += !require(l_mid > 0.0 && l_mid < 1.0, "lambda(short): acceleration phase progress");
    failures += !require(approx_equal(speed, profile.a * t_mid * 60.0), "lambda(short): acceleration speed");

    const double l_end = b.lambda(profile.dt, speed);
    failures += !require(approx_equal(l_end, 1.0), "lambda(short): at end of block");
    failures += !require(approx_equal(speed, 0.0), "lambda(short): speed at end of block");
  }

  // calc_arc(): center from I/J offsets and interpolation over a quarter circle
  {
    Block start("G1 X0.25 Y0 Z0 F1200");
    start.parse(&machine);

    Block arc("G3 X0 Y0.25 I-0.25 J0 F500", start);
    arc.parse(&machine);

    const double arc_limit = std::sqrt(machine.A() * arc.r()) * 60.0;
    const double arc_expected_feed = std::min(500.0, arc_limit);

    failures += !require(approx_equal(arc.center().x(), 0.0), "calc_arc(IJ): center x");
    failures += !require(approx_equal(arc.center().y(), 0.0), "calc_arc(IJ): center y");
    failures += !require(approx_equal(arc.r(), 0.25), "calc_arc(IJ): radius");
    failures += !require(approx_equal(arc.theta_0(), 0.0), "calc_arc(IJ): initial angle");
    failures += !require(approx_equal(arc.dtheta(), std::acos(-1.0) / 2.0), "calc_arc(IJ): swept angle");
    failures += !require(approx_equal(arc.length(), std::acos(-1.0) * 0.25 / 2.0), "calc_arc(IJ): arc length");
    failures += !require(approx_equal(arc.arc_feedrate(), arc_expected_feed), "calc_arc(IJ): arc feedrate limited");

    const Point p_half = arc.interpolate(0.5);
    const double c = std::sqrt(0.5) * 0.25;
    failures += !require(approx_equal(p_half.x(), c), "interpolate(arc IJ): x at lambda=0.5");
    failures += !require(approx_equal(p_half.y(), c), "interpolate(arc IJ): y at lambda=0.5");
    failures += !require(approx_equal(p_half.z(), 0.0), "interpolate(arc IJ): z stays constant");

    double speed = -1.0;
    const double l_end = arc.lambda(arc.dt(), speed);
    failures += !require(approx_equal(l_end, 1.0), "lambda(arc IJ): end of block");
    failures += !require(approx_equal(speed, 0.0), "lambda(arc IJ): speed at end of block");
  }

  // calc_arc(): radius-based center reconstruction and feedrate limiting
  {
    Block start("G1 X0.25 Y0 Z0 F2000");
    start.parse(&machine);

    Block arc("G3 X0 Y0.25 R0.25 F2000", start);
    arc.parse(&machine);

    const double arc_limit = std::sqrt(machine.A() * arc.r()) * 60.0;
    const double arc_expected_feed = std::min(2000.0, arc_limit);

    failures += !require(approx_equal(arc.center().x(), 0.0), "calc_arc(R): center x");
    failures += !require(approx_equal(arc.center().y(), 0.0), "calc_arc(R): center y");
    failures += !require(approx_equal(arc.r(), 0.25), "calc_arc(R): radius");
    failures += !require(approx_equal(arc.dtheta(), std::acos(-1.0) / 2.0), "calc_arc(R): swept angle");
    failures += !require(approx_equal(arc.arc_feedrate(), arc_expected_feed), "calc_arc(R): limited arc feedrate");
    failures += !require(approx_equal(arc.length(), std::acos(-1.0) * 0.25 / 2.0), "calc_arc(R): arc length");
  }

  // parse() with arc endpoint mismatch should throw
  {
    Block start("G1 X0.25 Y0 Z0 F1200");
    start.parse(&machine);

    Block arc("G3 X0 Y0.25 I-0.25 J0.05 F1200", start);
    bool threw = false;
    try {
      arc.parse(&machine);
    } catch (const std::runtime_error &) {
      threw = true;
    }
    failures += !require(threw, "calc_arc(IJ): endpoint mismatch throws runtime_error");
  }

  // constructor with previous block + operator=
  {
    Block b1("N10 G1 X1 Y2 Z3 F100 S200 T1 M3");
    b1.parse(&machine);

    Block b2("G1 X4 Y5 Z6", b1);
    failures += !require(b2.prev == &b1, "ctor(line, prev): sets prev link");
    failures += !require(b1.next == &b2, "ctor(line, prev): sets next link on previous block");

    b2.parse(&machine);
    failures += !require(b2.n() == b1.n() + 1, "operator=: inherited incremented block number");
    failures += !require(approx_equal(b2.feedrate(), b1.feedrate()), "operator=: inherited feedrate");
    failures += !require(approx_equal(b2.spindle(), b1.spindle()), "operator=: inherited spindle");
    failures += !require(b2.tool() == b1.tool(), "operator=: inherited tool");
  }

  // parse() with malformed token should throw
  {
    Block b("G1 X10 Q77");
    bool threw = false;
    try {
      b.parse(&machine);
    } catch (const std::runtime_error &) {
      threw = true;
    }
    failures += !require(threw, "parse(): unsupported token throws runtime_error");
  }

  // walk() callback should execute at least once (profile dt defaults to 0 -> one step)
  {
    Block b("G1 X1 Y1 Z1 F100");
    b.parse(&machine);

    int calls = 0;
    b.walk([&calls](Block &blk, double t, double lam, double spd) {
      (void)blk;
      (void)t;
      (void)lam;
      (void)spd;
      ++calls;
    });

    failures += !require(calls >= 1, "walk(): callback invoked");
  }

  if (failures == 0) {
    std::cout << "All Block tests passed.\n";
  } else {
    std::cout << failures << " Block test(s) failed.\n";
  }

  return failures;
}
