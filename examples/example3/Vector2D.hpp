#pragma once

#include <cstddef>
#include <ostream>
#include <string>

/// @class Vector2D
/// @brief Represents a 2D vector with a textual label.
///
/// The class stores two Cartesian components (x, y) and a label.
/// It provides basic vector operations, distance/norm utilities,
/// CSV serialization helpers, and stream printing support.
class Vector2D {
public:
  /// @brief Constructs a vector with empty label and zero components.
  Vector2D();

  /// @brief Constructs a vector with the given components and empty label.
  /// @param x X component.
  /// @param y Y component.
  Vector2D(double x, double y);

  /// @brief Constructs a vector with label and components.
  /// @param label Vector label.
  /// @param x X component.
  /// @param y Y component.
  Vector2D(const std::string label, double x, double y);

  /// @brief Sets the vector label.
  /// @param label New label value.
  void set_label(std::string label);

  /// @brief Sets both vector components.
  /// @param x New X component.
  /// @param y New Y component.
  void set(double x, double y);

  /// @brief Returns the X component.
  /// @return Current X component.
  double x() const;

  /// @brief Returns the Y component.
  /// @return Current Y component.
  double y() const;

  /// @brief Returns the vector label.
  /// @return Current label.
  std::string label() const;

  /// @brief Computes the Euclidean norm.
  /// @return Value of sqrt(x^2 + y^2).
  double norm() const;

  /// @brief Computes Euclidean distance from another vector.
  /// @param other Vector to measure distance from.
  /// @return Euclidean distance between this vector and other.
  double distance_to(const Vector2D& other) const;

  /// @brief Returns the component-wise sum with another vector.
  /// @param other Vector to add.
  /// @return A new vector equal to this + other.
  Vector2D operator+(const Vector2D& other) const;

  /// @brief Returns the component-wise difference with another vector.
  /// @param other Vector to subtract.
  /// @return A new vector equal to this - other.
  Vector2D operator-(const Vector2D& other) const;

  /// @brief Returns this vector scaled by a scalar.
  /// @param scalar Scale factor.
  /// @return A new vector equal to this * scalar.
  Vector2D operator*(double scalar) const;

  /// @brief Returns the CSV header string.
  /// @return Header in CSV format.
  static std::string to_csv_header();

  /// @brief Returns this vector serialized as one CSV row.
  /// @return Row in CSV format.
  std::string to_csv_row() const;
  
  /// @brief Streams a textual representation of the vector.
  /// @param os Output stream.
  /// @param v Vector to stream.
  /// @return Reference to the output stream.
  friend std::ostream& operator<<(std::ostream& os, const Vector2D& v);

private:
  std::string _label;
  double _x;
  double _y;
  static size_t _csv_counter;
};


