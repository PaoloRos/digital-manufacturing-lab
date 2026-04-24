/*
  ____                                        ____        __ _                 
 / ___|___  _ __ ___  _ __ ___   ___  _ __   |  _ \  ___ / _(_)_ __   ___  ___ 
| |   / _ \| '_ ` _ \| '_ ` _ \ / _ \| '_ \  | | | |/ _ \ |_| | '_ \ / _ \/ __|
| |__| (_) | | | | | | | | | | | (_) | | | | | |_| |  __/  _| | | | |  __/\__ \
 \____\___/|_| |_| |_|_| |_| |_|\___/|_| |_| |____/ \___|_| |_|_| |_|\___||___/
                              

Author: Paolo Rossi
Created: 2026-04-14
*/

#pragma once  // needs modern C++ (since C++11)

#include <optional>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

// Semantic versioning: main.minor.patch, where patch is for bug fixes, minor
// for new features, and main for breaking changes. See https://semver.org/ for
// more details.
#define CNCPP_VERSION_ "0.1.0"

// Number of digits reserved for printing numbers
#define CNCPP_NUMBERS_WIDTH "9"


// Custom type for data values
using data_t = double;
// A point in ISO Gcode may have a coordinate or not: so use an optional type
using opt_data_t = std::optional<data_t>;
using opt_int_t = std::optional<int>;

namespace cncpp {
/**
 * @brief Common base interface for printable CNC domain objects.
 */
class Object {
public:
  /**
   * @brief Build a textual description of the object.
   * @param colored Enable ANSI colors when true.
   * @return Human-readable object description.
   */
  virtual std::string desc(bool colored = true) const = 0;

  /**
   * @brief Stream output operator.
   * @param os Output stream.
   * @param p Point to print.
   * @return Output stream reference.
   */
  friend 
  std::ostream& operator<<(std::ostream &os, Object const &p);

};

/**
 * @brief Stream output operator for Object.
 * @param os Output stream.
 * @param o Object to print.
 * @return Output stream reference.
 */
inline std::ostream& operator<<(std::ostream &os, Object const &o)
{
  bool is_terminal = (&os == &std::cout && isatty(STDOUT_FILENO)) ||
                    (&os == &std::cerr && isatty(STDERR_FILENO));

  os << o.desc(is_terminal);  // colored only if is terminal

  return os;
}

} // namespace cncpp