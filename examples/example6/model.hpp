#pragma once

#include "types.hpp"

#include <fstream>
#include <string>

class Model 
{
  public:

    // Constructor: the user has to specify the name of the model, the number of states and the number of inputs.
    // By default, the state is initialized to a zero vector.
    Model( std::string name, size_t n_states, size_t n_inputs );
    
    // Set the initial state of the model
    void set_x0( Vec initial_state );

    // Integrate the model for one time step
    void step( double dt, Vec inputs );

    bool start_log( std::string path );
    void stop_log();
    void csv_header( std::ostream& ) const;
    void csv_row( std::ostream& ) const;

  protected:
    std::string _name; // name of the model

    virtual Vec compute_x_dot_impl(
      double dt, Vec inputs, Vec states ) = 0; // pure virtual function, must be implemented by child classes

  private:
    // Implemented by model, but overridden by child classes.
    Vec compute_x_dot( double dt, Vec inputs, Vec states );

    size_t _n_states;
    size_t _n_inputs;
    std::ofstream _log_file; // file for logging
    
    Vec _states;
    double _t;
};

// Miglioria: prova a fare il calcolo delal derivatya come una un membro const
// Altra miglioria: metti riferimento in comput xdot, anziche fare la copia