/*
 ____  _            _      _   _       _ _     _____         _   
| __ )| | ___   ___| | __ | | | |_ __ (_) |_  |_   _|__  ___| |_ 
|  _ \| |/ _ \ / __| |/ / | | | | '_ \| | __|   | |/ _ \/ __| __|
| |_) | | (_) | (__|   <  | |_| | | | | | |_    | |  __/\__ \ |_ 
|____/|_|\___/ \___|_|\_\  \___/|_| |_|_|\__|   |_|\___||___/\__|
                                      

Covers the public API of cncpp::Block and reports method-level failures.
*/

#include "block.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr double kEps = 1e-9;

bool approx_equal(double a, double b)
{
  return std::fabs(a - b) <= kEps;
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
