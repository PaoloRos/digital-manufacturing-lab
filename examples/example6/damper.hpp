#pragma once

#include "model.hpp"

class Damper : public Model
{
  public:

    enum States {
      STATE_X,
      STATE_V,
      STATES_NUMBER
    };

    enum Inputs { INPUTS_NUMBER };

    Damper( std::string name ) 
    : Model(name, STATES_NUMBER, INPUTS_NUMBER) {} // 2 states (position and velocity), 0 input (force)

  protected:

    Vec compute_x_dot_impl( double& dt, Vec& inputs, Vec& states ) override;

    ConfigErrors set_config( const std::map<std::string, double>& config ) override;

    std::optional< std::map<std::string, double> > get_config() const override
      { return std::map<std::string, double>{ {"c", _c}, {"m", _m} }; }
  
  private:
    double _c = 10.0;  // Damping coefficient N*s/m
    double _m = 3.0;  // Mass kg

};

// Nota sul damper sicuro dovrari modificare la lettura del file di input!

// fai che la risposta forzata è ereditata da quella libera (?)

// la massa è una proprietà del modello, non della molla o del damper



