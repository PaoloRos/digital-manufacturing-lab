/*
3D Point representation: its coordinates and useful methods to manipulate it.

Author: Paolo Rossi
Created: 2024-04-14
*/

#pragma once

#include "defines.hpp"

#include <vector>

// Namespace is a way to avoid name collisions between different libraries, typically a namespace is NOT indented (the only case of NO indentation)

namespace cncpp {

// 3D spatial Point class
class Point {
  
public:

  // LIFECYCLE (creating/destroying objects)

  // Point constructor: it can be called with 0, 1, 2, or 3 arguments, thanks to default values (otherwise would be redeundant defining an argument as std::nullopt, since the attributes are already initialized with nullopt)
  Point(opt_data_t x = std::nullopt, opt_data_t y = std::nullopt, opt_data_t z = std::nullopt);

  // Print a description of the point, with optional colored output (using fmt library)
  std::string desc(bool colored = true) const;
  
  // Reset the point to an empty state (all coordinates undefined)
  void reset();

  // OPERATORS/OPERATIONS

  // Compute the distance between two points (the difference between their coordinates)
  Point delta(Point const &o) const;
  // Compute the length of a point (the distance from the origin)
  data_t length() const;
  // Compute the modal coordinates between two points (see the notes on Notability): if 'this' (new instruction) has an empty coordinate, it'll be updated with the value of 'other' (previous instruction)
  void modal(Point const &o);

  Point& operator=(Point const &o); // 'this' = 'other' as reference
  Point operator+(Point const& o) const; // p3 = p1 + p2 as value

  // Check if the point is complete (all coordinates defined)
  bool is_complete() const { return _x && _y && _z; }

  // ACCESSORS

  data_t x() const { return _x.value(); }  // Copilot suggest to use value_or(0.0) -> perché? Se uso value_or(0.0) non riesco a distinguere tra un punto con x=0.0 e un punto con x non definito, mentre con value() se x non è definito viene lanciata un'eccezione std::bad_optional_access, che è più chiara per il debug: CHIEDI AL PROF!
  data_t y() const { return _y.value(); }
  data_t z() const { return _z.value(); }
  
  data_t x(data_t v) { return (_x = v).value(); }
  data_t y(data_t v) { return (_y = v).value(); }
  data_t z(data_t v) { return (_z = v).value(); }
  
  std::vector<data_t> vec() const;
  
private:
  opt_data_t _x = std::nullopt;
  opt_data_t _y = std::nullopt;
  opt_data_t _z = std::nullopt;
};



}; // namespace cncpp


// ====================== Annotation ======================

// Note about inline functions: they are a hint to the compiler to optimize the code by replacing the function call with the function body. This can improve performance for small functions, but it is not guaranteed that the compiler will actually inline the function. 
// Nowadays, modern compilers are very good at optimizing code, so the inline keyword is often not necessary. However, it can still be useful for small functions that are called frequently, as it can reduce the overhead of function calls. In this case, since the accessor functions are very simple and likely to be called frequently, it makes sense to declare them as inline.