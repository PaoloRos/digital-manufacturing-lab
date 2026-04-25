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
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

#include <fmt/color.h>
#include <fmt/format.h>

/**
 * @def CNCPP_VERSION_
 * @brief Project semantic version in main.minor.patch format.
 *
 * Patch is for bug fixes, minor for new features, and main for breaking
 * changes. See https://semver.org/ for details.
 */
#define CNCPP_VERSION_ "0.1.0"

/**
 * @def CNCPP_NUMBERS_WIDTH
 * @brief Number of digits reserved for printing numeric values.
 */
#define CNCPP_NUMBERS_WIDTH "9"


/**
 * @brief Scalar numeric type used for CNC data values.
 */
using data_t = double;

/**
 * @brief Optional scalar value for coordinates that may be omitted in G-code.
 */
using opt_data_t = std::optional<data_t>;

/**
 * @brief Optional integer type used for nullable integral parameters.
 */
using opt_int_t = std::optional<int>;

namespace cncpp {

/**
 * @brief Log categories used for terminal output.
 */
enum class LogType {
  Message,
  Computation,
  Warning,
  Error,
};

/**
 * @brief Build a bold colored tag for terminal logging.
 * @param type Log category.
 * @param os Output stream to validate as terminal.
 * @return Formatted tag string like "[Message]".
 * @throws std::runtime_error If the stream is not a terminal stream.
 */
inline std::string log_tag(LogType type, std::ostream &os = std::cerr)
{
  bool const is_terminal = (&os == &std::cout && isatty(STDOUT_FILENO)) ||
                           (&os == &std::cerr && isatty(STDERR_FILENO));

  if (!is_terminal) {
    throw std::runtime_error("cncpp::log_tag works only with terminal output streams");
  }

  switch (type) {
  case LogType::Message:
    return fmt::format(fmt::fg(fmt::color::green) | fmt::emphasis::bold, "[Message]");
  case LogType::Computation:
    return fmt::format(fmt::fg(fmt::color::blue) | fmt::emphasis::bold, "[Computation]");
  case LogType::Warning:
    return fmt::format(fmt::fg(fmt::color::gold) | fmt::emphasis::bold, "[Warning]");
  case LogType::Error:
    return fmt::format(fmt::fg(fmt::color::red) | fmt::emphasis::bold, "[Error]");
  default:
    throw std::runtime_error("Unsupported cncpp::LogType value");
  }
}

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

  friend 
  std::ostream &operator<<(std::ostream &os, Object const &p);

};

/**
 * @brief Stream output operator for Object.
 * @details Delegates formatting to the virtual Object::desc(bool) method,
 * selecting colored output only when the target stream is a terminal.
 * @param os Output stream.
 * @param o Object to print.
 * @return Output stream reference.
 */
inline std::ostream &operator<<(std::ostream &os, Object const &o)
{
  bool is_terminal = (&os == &std::cout && isatty(STDOUT_FILENO)) ||
                     (&os == &std::cerr && isatty(STDERR_FILENO));

  os << o.desc(is_terminal);  // colored only if is terminal

  return os;
}

} // namespace cncpp