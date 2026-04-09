#pragma once

#include "types.hpp"

#include <fstream>
#include <string>
#include <exception>
#include <map>

std::string trim(std::string str);

class Model 
{
  public:

    // Constructor: the user has to specify the name of the model, the number of states and the number of inputs.
    // By default, the state is initialized to a zero vector.
    Model( std::string name, size_t n_states, size_t n_inputs );

    // Parsing config files (key-value pairs)
    bool load_config( std::string path );
    // Saving config files (key-value pairs)
    bool save_config( std::string path ) const;
    
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

    virtual Vec compute_x_dot_impl( double dt, Vec inputs, Vec states ) = 0;  // pure virtual function, it MUST be implemented by child classes
    
    virtual bool set_config( std::map<std::string, double> config ) { return true; } // default implementation: it's not necessary to implement it in child classes, but it can be overridden if needed
    
    virtual std::optional<
    std::map<std::string, double>
    > get_config() const { return {}; } // Return an empty map by default, but it can be overridden by child classes to return the current config values

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

// miglioria: implementa un enum per ritornare l'errore specifico in set_config, anziche un bool (es: enum class ConfigError { NONE, INVALID_VALUE, MISSING_KEY, ... };)
/*
eunm class LoadConfigError {
  NONE,
  FILE_NOT_FOUND,
  INVALID_FORMAT,
  MISSING_KEY,
  INVALID_VALUE
};
LoadConfigError err = LoadConfigError::NONE;

// The main use of enum class is to provide a type-safe way to represent a set of related constants, such as error codes, states, or options. By using an enum class, you can avoid issues with implicit conversions and improve code readability. In the context of the load_config function, using an enum class for error handling allows you to clearly indicate the specific error that occurred during the loading process, making it easier for developers to understand and handle different error scenarios appropriately.

vedi implementazione di pippo su github

*/

// std::optional usata per integrarte valore booleano e valore di una variabile... due cose in una