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