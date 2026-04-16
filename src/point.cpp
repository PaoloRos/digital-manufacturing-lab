/*
Point class implementation file.

Author: Paolo Rossi
Created: 2024-04-14
*/

#include "point.hpp"

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/color.h>

using namespace std;  // generally in the implementation file is acceptable
using namespace cncpp;
using namespace fmt;
using col_t = optional<fmt::color>;

// STATIC DECLARATIONS

static string coord_str(opt_data_t const &coord, col_t const &color = nullopt);

// LYFECYCLE

Point::Point(opt_data_t x, opt_data_t y, opt_data_t z) : _x(x), _y(y), _z(z) {}

string Point::desc(bool colored) const
{
  stringstream ss;
  ss  << "[" 
      << coord_str(_x, colored ? col_t(color::red) : nullopt) << ", "
      << coord_str(_y, colored ? col_t(color::green) : nullopt) << ", " 
      << coord_str(_z, colored ? col_t(color::blue) : nullopt) 
      << "]";
  return ss.str();
}

void Point::reset()
{
  _x.reset();
  _y.reset();
  _z.reset();
}

// OPERATORS/OPERATIONS

Point Point::delta(Point const &o) const 
{
  if(!is_complete() || !o.is_complete()) {
    throw runtime_error("Cannot compute delta: incomplete points!");
  }
  return Point(_x.value() - o._x.value(), _y.value() - o._y.value(), _z.value() - o._z.value());
}

data_t Point::length() const 
{
  if(!is_complete()) {
    throw runtime_error("Cannot compute length: incomplete point!");
  }
  return hypot(_x.value(), _y.value(), _z.value());
}

void Point::modal(Point const &o) 
{
  // Modal coords: inheritance of previous coords
  if(o._x && !_x) _x = o._x;
  if(o._y && !_y) _y = o._y;
  if(o._z && !_z) _z = o._z;
}

Point& Point::operator=(Point const &o) 
{
  // return *this = Point(o._x, o._y, o._z); // it's expensive to create a temporary object, since a reference to 'this' is rerturned
  _x = o._x;
  _y = o._y;
  _z = o._z;
  return *this;
}

Point Point::operator+(Point const& o) const 
{
  if(!is_complete() || !o.is_complete()) {
    throw runtime_error("Cannot compute sum: incomplete points!");
  }
  return Point(_x.value() + o._x.value(), _y.value() + o._y.value(), _z.value() + o._z.value());
}

// ACCESSORS

std::vector<data_t> Point::vec() const 
{
  if(!is_complete()) {
    throw runtime_error("Cannot convert to vector: incomplete point!");
  }
  return {_x.value(), _y.value(), _z.value()};
}

std::ostream &cncpp::operator<<(std::ostream &os, Point const &p) 
{
  bool is_terminal = false;

  if (os.rdbuf() == std::cout.rdbuf()) {
    is_terminal = (isatty(STDOUT_FILENO) == 1);
  } else if (os.rdbuf() == std::cerr.rdbuf() || os.rdbuf() == std::clog.rdbuf()) {
    is_terminal = (isatty(STDERR_FILENO) == 1);
  }

  os << p.desc(is_terminal);  // colored only if is terminal

  return os;
}

// STATIC DEFINITIONS

static string coord_str(opt_data_t const &coord, col_t const &color) 
{
  string str;
  if (coord && color) {
    str = format( "{:" CNCPP_NUMBERS_WIDTH ".3f}", styled(coord.value(), fg(color.value())) );  // styled() allows to apply a text style to a value, and fg() creates a text style with a foreground color
  } else if (coord) {
    str = format("{:" CNCPP_NUMBERS_WIDTH ".3f}", coord.value());  // just number
  } else {
    str = format( "{:>" CNCPP_NUMBERS_WIDTH "}", "-" ); // 8 spaces + '-', to align with the other coordinates
  }
  return str;
}

// TESTS

// In cpp only one main: if we want to write tests in the same file, we can use a preprocessor directive to include a main function only when a specific macro is defined (e.g., CNCPP_TEST_MAIN). This way, we can compile the file with tests when needed, and without tests otherwise.
#ifdef CNCPP_TEST_MAIN



int main() {
  Point p1(1.0,2.0,3.0);
  cout << "p1: " << p1 << endl;

  Point p2(4.0,5.0); // z is undefined
  cout << "p2: " << p2 << endl;
  p2.modal(p1); // p2 inherits z from p1
  cout << "p2 after modal: " << p2 << endl;

  Point p3 = p1 + p2; // p3 is the sum of p1 and p2
  cout << "p3 = p1 + p2: " << p3 << endl;
  cout << "Length of p3: " << p3.length() << endl;
  cerr << "Delta p3 - p1: " << p3.delta(p1) << endl;

  return 0;
}

#endif // CNCPP_TEST_MAIN

// ========= ANNOTATIONS ========

// audio: la migliore maniera per salvare un file è leggerlo senza salvarlo:
// it's better to read directly from output
// but a lot of garbage (console formatting)
// solve that problem
//

// stderror: not in the buffer -> not redirected to the file, but directly to the console, so we can see it immediately without opening the file
// stdoutput: redirected to the file, so we can save it and analyze it later without the garbage of the console formatting (in the buffer)
