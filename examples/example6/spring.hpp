#pragma once

#include "model.hpp"

#include <iostream>
#include <map>

class Spring : public Model
{
  public:

    // Enumeration for the states, to make it easier to read the code.
    // Benefits: I can read the code easily and I can add more states without
    // changing the rest of the code. For example, if I want to add a state for
    // the acceleration, I can just add it to the enum and update the
    // compute_x_dot_impl function.
    enum States {
      STATE_X,
      STATE_V,
      STATES_NUMBER // not a state, but it's useful to know how many states I have
    };

    enum Inputs { INPUTS_NUMBER };

    Spring( std::string name ) 
    : Model(name, STATES_NUMBER, INPUTS_NUMBER) {} // 2 states (position and velocity), 0 input (force)

  protected:

    Vec compute_x_dot_impl( double dt, Vec inputs, Vec states ) override;

    bool set_config( std::map<std::string, double> config) override;

    std::optional<
    std::map<std::string, double>
    > get_config() const override
      { return std::map<std::string, double>{ {"k", _k}, {"m", _m} }; }
  
  private:
    double _k = 100.0;  // Spring stiffness N/m
    double _m = 3.0;  // Mass kg

};

Vec Spring::compute_x_dot_impl(double dt, Vec inputs, Vec states)
{
  Vec x_dot = states;

  x_dot[STATE_X] = states[STATE_V];
  x_dot[STATE_V] = - states[STATE_X] * _k/_m; // F = m*a = -k*x <=> a = - k*x/m

  return x_dot;
}

bool Spring::set_config(std::map<std::string, double> config) 
{
  bool ret = true;  // returned value
  if(config.find("k") != config.end()) {

    _k = config["k"];
  } else { 
    std::cerr << "Warning: 'k' not found in config.\n";
    ret = false;
  }
  if(config.find("m") != config.end()) {

    _m = config["m"];
  } else { 
    std::cerr << "Warning: 'm' not found in config.\n";
    ret = false;
  }

  return ret;
  }

