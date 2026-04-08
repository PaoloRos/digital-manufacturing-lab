#pragma once

#include "model.hpp"

class ForcedSpring : public Model
{
  public:
    enum States {
      STATE_X,
      STATE_V,
      STATES_NUMBER,
    };

    enum Inputs {
      INPUT_F,
      INPUTS_NUMBER,
    };

    ForcedSpring( std::string name )
    : Model(name, STATES_NUMBER, INPUTS_NUMBER) {}
    
  protected:
    Vec compute_x_dot_impl( double dt, Vec inputs, Vec states ) override;

  private:
    double _k = 100.0;  // Spring stiffness N/m
    double _m = 3.0;  // Mass kg
};

Vec ForcedSpring::compute_x_dot_impl(double dt, Vec inputs, Vec states) 
{
  Vec x_dot(states.size());
  
  x_dot[STATE_X] = states[STATE_V];
  x_dot[STATE_V] = (inputs[INPUT_F] - states[STATE_X] * _k) / _m; // F = m*a = F - k*x <=> a = (F - k*x)/m

  return x_dot;
}