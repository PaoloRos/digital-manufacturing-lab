#pragma once

#include "types.hpp"

#include <fstream>
#include <map>

// Error definition about model configuration
enum class ConfigErrorCode {
  NONE,
  MISSING_KEY,
  INVALID_VALUE,
  OUT_OF_RANGE,
  UNKNOWN_KEY,
};

// Error container about model configuration
struct ConfigError {
  ConfigErrorCode code = ConfigErrorCode::NONE; // setted by default
  std::string key;
  std::optional<double> value;
};

// Why a vector of ConfigError? Because there could be multiple errors in the
// config file, and we want to report all of them to the user, instead of just
// the first one. Go to /spring.hpp->set_config for an example of how to use it.
using ConfigErrors = std::vector<ConfigError>;

class Model 
{
  public:

    // Constructor: the user has to specify the name of the model, the number of states and the number of inputs.
    // By default, the state is initialized to a zero vector.
    Model( std::string name, size_t n_states, size_t n_inputs );

    // Parsing config files (key-value pairs)
    void load_config( std::string& path );
    // Saving config files (key-value pairs)
    void save_config( std::string& path ) const;
    
    // Set the initial state of the model
    void set_x0( Vec initial_state );

    // Integrate the model for one time step
    void step( double dt, Vec inputs );

    bool start_log( std::string& path );
    inline void stop_log() { _log_file.close(); }
    void csv_header( std::ostream& os ) const;
    void csv_row( std::ostream& os ) const;

    inline std::string get_name() const { return _name; }

  protected:

    std::string _name; // name of the model

    virtual Vec compute_x_dot_impl( double& dt, Vec& inputs, Vec& states ) = 0;  // pure virtual function, it MUST be implemented by child classes
    
    virtual ConfigErrors set_config( const std::map<std::string, double>& config ) 
      { return {}; } // default implementation: it's not necessary to implement it in child classes, but it can be overridden if needed
    
    // Return the current config values as a map of key-value pairs. By
    // default, it returns an empty map, but it can be overridden by child
    // classes to return the actual config values.
    // std::optional is a an object that contains another object, allowing to return
    // a boolean value whether the container is empty or not
    virtual std::optional< std::map<std::string, double> > get_config() const 
      { return std::nullopt; }

  private:

    // Implemented by model, but overridden by child classes.
    Vec compute_x_dot( double& dt, Vec& inputs, Vec& states );

    size_t _n_states;
    size_t _n_inputs;
    std::ofstream _log_file; // file for logging
    
    Vec _states;
    double _t;
};