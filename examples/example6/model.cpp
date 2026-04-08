#include "model.hpp"

#include <exception>

Model::Model(std::string name, size_t n_states, size_t n_inputs)
: _name(name), _n_states(n_states), _n_inputs(n_inputs) {

  _states.resize(_n_states, 0.0); // initialize state to zero
}

void Model::set_x0(Vec initial_state) {

  if(_states.size() != initial_state.size()) {
    
    throw std::invalid_argument("Invalid state size!");
  }
  _states = initial_state;
}

void Model::step(double dt, Vec inputs) {

  Vec x_dot = compute_x_dot(dt, inputs, _states);
  for (size_t i = 0; i < _n_states; i++) {

    _states[i] += dt * x_dot[i];  // Euler integration (as in Simulink)
  }
  _t += dt;

  csv_row(_log_file);
}

Vec Model::compute_x_dot(double dt, Vec inputs, Vec states) {
  
  if (dt <= 0) {

    throw std::invalid_argument("Invalid dt: must be positive!");
  }

  if (inputs.size() != _n_inputs) {

    throw std::invalid_argument("Invalid input size!");
  }

  return compute_x_dot_impl(dt, inputs, states);
}

bool Model::start_log(std::string path) {

  std::string file_name = path + "/" + _name + ".csv";

  _log_file.open(file_name);
  if (!_log_file.is_open()) {

    throw std::runtime_error("Could not open log file: " + file_name);
  }
  csv_header(_log_file);
  return true;
}

void Model::stop_log() {
  _log_file.close();
}

void Model::csv_header(std::ostream& os) const {

  os << "t";
  for(size_t i = 0; i < _n_states; i++) {
    os << "," << "x" << i;
  }
  os << std::endl;
}

void Model::csv_row(std::ostream& os) const {

  os << _t;
  for(size_t i = 0; i < _n_states; i++) {
    os << "," << _states[i];
  }
  os << std::endl;
}