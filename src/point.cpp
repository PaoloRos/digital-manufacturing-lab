/*
Point class implementation file.

Author: Paolo Rossi
Created: 2024-04-14
*/

#include "point.hpp"

#include <sstream>
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
      << coord_str(_x, col_t(color::red)) << ", " 
      << coord_str(_y, col_t(color::green)) << ", " 
      << coord_str(_z, col_t(color::blue)) 
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


// STATIC DEFINITIONS

static string coord_str(opt_data_t const &coord, col_t const &color) 
{
  string str;
  if (coord && color) {
    str = format( "{:" CNCPP_NUMBERS_WIDTH ".3f}", styled(coord.value(), fg(color.value())) );  // styled() allows to apply a text style to a value, and fg() creates a text style with a foreground color
  } else if (coord) {
    str = format("{:" CNCPP_NUMBERS_WIDTH ":.3f}", coord.value());  // just number
  } else {
    str = format( "{:>" CNCPP_NUMBERS_WIDTH "}", "-" ); // 8 spaces + '-', to align with the other coordinates
  }
  return str;
}