#include "spring.hpp"
#include "forced_spring.hpp"

#include <iostream>
#include <fmt/format.h>
#include <rang.hpp>


int main()
{
  // Usage of fmt and rang libraries fpr nice console output
  std::cout << fmt::format("{:^40}", "CUSTOM SIMULINK") << std::endl; // string centered in a field of 40 characters
  std::cout << fmt::format("{:^40}", "v1.0") << std::endl;
  std::cout << rang::bg::green << "Starting simulation..." << rang::bg::reset << std::endl;

  Spring spring("spring");
  ForcedSpring fspring_1("constant_force");
  ForcedSpring fspring_2("sinusoidal_force");

  spring.start_log("./examples/example6/out");
  fspring_1.start_log("./examples/example6/out");
  fspring_2.start_log("./examples/example6/out");

  spring.set_x0({1.0, 0.0}); // initial position = 1.0, initial velocity = 0.0
  fspring_1.set_x0({0.0, 0.0});
  fspring_2.set_x0({0.0, 0.0});

  // Model simulation
  double tf = 20;
  double dt = 0.01;
  for(double t = 0.0; t < tf; t += dt) {

    spring.step(dt, {}); // no inputs
    fspring_1.step(dt, {1.0});
    fspring_2.step(dt, {1.0 + std::sin(t * 2 * M_PI * 10.0)});
  }

  std::cout << rang::bg::green << "Simulation terminated!" << rang::bg::reset << std::endl;

  return 0;
}