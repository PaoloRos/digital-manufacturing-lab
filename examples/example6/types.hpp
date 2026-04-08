#pragma once

#include <vector>
#include <memory>

// Define alias trhough `using` instead of `typedef` since it is more readable and more powerful (it can be used with templates).

// Define an alias for a vector of doubles
using Vec = std::vector<double>;

// Define an alias for a shared pointer to a vector<T>
// How to use it: s_ptr<double>
template <typename T>
using s_ptr = std::shared_ptr<T>;

