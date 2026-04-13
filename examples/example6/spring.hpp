#pragma once

#include "model.hpp"

#include <iostream>
#include <map>
#include <rang.hpp>

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

    Vec compute_x_dot_impl( double& dt, Vec& inputs, Vec& states ) override;

    ConfigErrors set_config( const std::map<std::string, double>& config ) override;

    std::optional< std::map<std::string, double> > get_config() const override
      { return std::map<std::string, double>{ {"k", _k}, {"m", _m} }; }
  
  private:
    double _k = 100.0;  // Spring stiffness N/m
    double _m = 3.0;  // Mass kg

};

Vec Spring::compute_x_dot_impl(double& dt, Vec& inputs, Vec& states)
{
  Vec x_dot = states;

  x_dot[STATE_X] = states[STATE_V];
  x_dot[STATE_V] = - states[STATE_X] * _k/_m; // F = m*a = -k*x <=> a = - k*x/m

  return x_dot;
}

//bool Spring::set_config(const std::map<std::string, double>& config) 
//{
//  bool ret = true;  // returned value
//  
//  if(config.find("k") != config.end()) {
//    _k = config["k"];
//  } else { 
//    std::cerr << rang::style::bold << rang::fg::yellow
//              << "Warning: "
//              << rang::style::reset << rang::fg::reset
//              << "'k' not found in 'config'.\n";
//    ret = false;
//  }
//
//  if(config.find("m") != config.end()) {
//    _m = config["m"];
//  } else { 
//    std::cerr << rang::style::bold << rang::fg::yellow
//              << "Warning: "
//              << rang::style::reset << rang::fg::reset
//              << "'m' not found in 'config'.\n";
//    ret = false;
//  }
//
//  return ret;
//  }

ConfigErrors Spring::set_config(const std::map<std::string, double>& config)
{
  ConfigErrors errors;

  // Setting stiffness k
  auto k_it = config.find("k");
  if (k_it == config.end()) { 
    errors.push_back( {ConfigErrorCode::MISSING_KEY, "k", std::nullopt} );
  } else if (k_it->second <= 0.0) { 
    errors.push_back( {ConfigErrorCode::INVALID_VALUE, "k", k_it->second} );
  } else{ 
    _k = k_it->second; 
  }

  // Setting mass m
  auto m_it = config.find("m");
  if (m_it == config.end()) { 
    errors.push_back( {ConfigErrorCode::MISSING_KEY, "m", std::nullopt} );
  } else if (m_it->second <= 0.0) { 
    errors.push_back( {ConfigErrorCode::INVALID_VALUE, "m", m_it->second} );
  } else{ 
    _m = m_it->second; 
  }

  return errors;
}