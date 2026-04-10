#include "spring.hpp"
#include "forced_spring.hpp"

#include <iostream>
#include <fmt/format.h>
#include <rang.hpp>


int main(int argc, char** argv)
{
  std::cout << argc << " ";
  for(size_t i = 0; i < argc; i++)
    { std::cout << argv[i] << " "; }
  std::cout << std::endl;

  
  std::string out_folder; //= "./examples/example6/out/";
  std::string config_folder; //= "./examples/example6/configs/";
  
  if (argc > 1) { out_folder = argv[1]; }
  if (argc > 2) { config_folder = argv[2]; }

  // Usage of fmt and rang libraries fpr nice console output
  std::cout << fmt::format("{:^40}", "CUSTOM SIMULINK") << std::endl; // string centered in a field of 40 characters
  std::cout << fmt::format("{:^40}", "v1.0") << std::endl;
  std::cout << rang::bg::green << "Starting simulation..." << rang::bg::reset << std::endl;

  Spring spring("spring");
  //ForcedSpring fspring_1("constant_force");
  //ForcedSpring fspring_2("sinusoidal_force");
  
  try {
    spring.load_config(config_folder);
    //fspring_1.load_config(config_folder);
    //fspring_2.load_config(config_folder);
  } catch (std::runtime_error& e){
    std::cerr << rang::style::bold << rang::fg::red << "Error"
              << rang::style::reset << rang::fg::reset
              << " loading config: " << e.what() << std::endl;
    return 1;
  }

  
  if(!spring.save_config(config_folder)) {
    std::cerr << rang::style::bold << rang::fg::yellow
              << "Warning: "
              << rang::style::reset << rang::fg::reset
              << "could not save config for model '" << spring.get_name() << "'.\n";
  }
  //fspring_1.save_config(config_folder);
  //fspring_2.save_config(config_folder);

  spring.start_log(out_folder);
  //fspring_1.start_log(out_folder);
  //fspring_2.start_log(out_folder);

  spring.set_x0({1.0, 0.0}); // initial position = 1.0, initial velocity = 0.0
  //fspring_1.set_x0({0.0, 0.0});
  //fspring_2.set_x0({0.0, 0.0});

  // Model simulation
  double tf = 20;
  double dt = 0.0001;
  for(double t = 0.0; t < tf; t += dt) {

    spring.step(dt, {}); // no inputs
    //fspring_1.step(dt, {1.0});
    //fspring_2.step(dt, {1.0 + std::sin(t * 2 * M_PI * 10.0)});

    //spring.csv_row(std::cout);
  }

  std::cout << rang::bg::green << "Simulation terminated!" << rang::bg::reset << std::endl;

  return 0;
}