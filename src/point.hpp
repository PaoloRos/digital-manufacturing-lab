/*
 ____       _       _          _               
|  _ \ ___ (_)_ __ | |_    ___| | __ _ ___ ___ 
| |_) / _ \| | '_ \| __|  / __| |/ _` / __/ __|
|  __/ (_) | | | | | |_  | (__| | (_| \__ \__ \
|_|   \___/|_|_| |_|\__|  \___|_|\__,_|___/___/
                           

3D Point representation: its coordinates and useful methods to manipulate it.

Author: Paolo Rossi
Created: 2024-04-14
*/

#pragma once

#include "defines.hpp"

#include <vector>
#include <string>
#include <ostream>

// Namespace is a way to avoid name collisions between different libraries, typically a namespace is NOT indented (the only case of NO indentation)

namespace cncpp {

/**
 * @brief 3D spatial point with optional coordinates.
 *
 * A Point can store each coordinate as defined or undefined. This is useful
 * for modal CNC semantics, where missing values can be inherited from previous
 * instructions.
 */
class Point : public Object {
  
public:

  // ====== LIFECYCLE (creating/destroying objects) =====

  /**
   * @brief Construct a point with optional coordinates.
   * @param x X coordinate. If std::nullopt, x is undefined.
   * @param y Y coordinate. If std::nullopt, y is undefined.
   * @param z Z coordinate. If std::nullopt, z is undefined.
   */
  Point(opt_data_t x = std::nullopt, opt_data_t y = std::nullopt, opt_data_t z = std::nullopt);

  /**
   * @brief Build a string description of the point.
   * @param colored Enable ANSI colored output when true.
   * @return Human-readable point description.
   */
  std::string desc(bool colored = true) const override;
  
  /**
   * @brief Reset all coordinates to undefined.
   */
  void reset();

  // ====== OPERATORS/OPERATIONS ======

  /**
   * @brief Compute coordinate-wise difference with another point.
   * @param o Other point.
   * @return Point equal to this - o for each coordinate.
   */
  Point delta(Point const &o) const;
  /**
   * @brief Compute Euclidean norm from the origin.
   * @return Point length.
   */
  data_t length() const;
  /**
   * @brief Apply modal completion using another point.
   * @param o Source point for missing coordinates.
   *
   * Undefined coordinates in this point are filled with values from o.
   */
  void modal(Point const &o);

  /**
   * @brief Copy assignment operator.
   * @param o Source point.
   * @return Reference to this point.
   */
  Point& operator=(Point const &o); // 'this' = 'other' as reference

  /**
   * @brief Coordinate-wise point sum.
   * @param o Other point.
   * @return New point equal to this + o.
   */
  Point operator+(Point const& o) const; // p3 = p1 + p2 as value

  /**
   * @brief Check whether all coordinates are defined.
   * @return True if x, y and z are all defined.
   */
  bool is_complete() const { return _x && _y && _z; }

  // ====== ACCESSORS ======

  /**
   * @brief Get x coordinate.
   * @return X coordinate value.
   * @throws std::bad_optional_access If x is undefined.
   */
  data_t x() const { return _x.value(); }

  /**
   * @brief Get y coordinate.
   * @return Y coordinate value.
   * @throws std::bad_optional_access If y is undefined.
   */
  data_t y() const { return _y.value(); }

  /**
   * @brief Get z coordinate.
   * @return Z coordinate value.
   * @throws std::bad_optional_access If z is undefined.
   */
  data_t z() const { return _z.value(); }
  
  /**
   * @brief Set x coordinate.
   * @param v New x value.
   * @return Assigned x value.
   */
  data_t x(data_t v) { return (_x = v).value(); }

  /**
   * @brief Set y coordinate.
   * @param v New y value.
   * @return Assigned y value.
   */
  data_t y(data_t v) { return (_y = v).value(); }

  /**
   * @brief Set z coordinate.
   * @param v New z value.
   * @return Assigned z value.
   */
  data_t z(data_t v) { return (_z = v).value(); }
  
  /**
   * @brief Convert point coordinates to vector form.
   * @return Vector containing point coordinates.
   */
  std::vector<data_t> vec() const;
  
private:
  /** @brief Optional X coordinate. */
  opt_data_t _x = std::nullopt;
  /** @brief Optional Y coordinate. */
  opt_data_t _y = std::nullopt;
  /** @brief Optional Z coordinate. */
  opt_data_t _z = std::nullopt;
};

}; // namespace cncpp


// ====================== Annotation ======================

// Note about inline functions: they are a hint to the compiler to optimize the code by replacing the function call with the function body. This can improve performance for small functions, but it is not guaranteed that the compiler will actually inline the function. 
// Nowadays, modern compilers are very good at optimizing code, so the inline keyword is often not necessary. However, it can still be useful for small functions that are called frequently, as it can reduce the overhead of function calls. In this case, since the accessor functions are very simple and likely to be called frequently, it makes sense to declare them as inline.