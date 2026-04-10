#include "model.hpp"

#include <exception>
#include <iostream>

// --- Local functions ---

// Erase leading and trailing spaces from a string
static void trim(std::string& str)
{
  str.erase(0, str.find_first_not_of(" "));
  str.erase(str.find_last_not_of(" ") + 1);
}

// ------------------------------

// --- Model class implementation ---

Model::Model(std::string name, size_t n_states, size_t n_inputs)
: _name(name), _n_states(n_states), _n_inputs(n_inputs) {

  _states.resize(_n_states, 0.0); // initialize state to zero
}

bool Model::load_config(std::string& path) 
{
  // <folder>/<model>.txt
  std::string file_name = path + "/" + _name + ".txt";
  std::ifstream config_file(file_name);
  if(!config_file.is_open()) {
    throw std::runtime_error("Could not open config file: " + file_name);
  }
  
  // 'map' is a kind of container that stores key-value pairs, where the keys are unique and ordered for values (red-black tree paradigm).
  std::map<std::string, double> config;
  //std::unordered_map<std::string, double> config; 
  // unordered map (hash map): for faster access, but ordered for keys (hast table paradigm)

  std::string line;
  while(std::getline(config_file, line)) {
    // k = 100.0
    // m = 20.0
    auto pos = line.find('=');
    if(pos == std::string::npos) {
      throw std::runtime_error("Config file format error: expected '<key>=<value>' format, but got: " + line);
    }

    std::string key = line.substr(0, pos);
    std::string value_str = line.substr(pos + 1);

    trim(key);
    trim(value_str);

    double value = std::stod(value_str);

    config[key] = value;
  }

  config_file.close();

  if(!set_config(config)) {
      throw std::runtime_error("Invalid config values in file: " + file_name);
    }
  return true;
}

bool Model::save_config(std::string& path) const
{
  auto maybe_config = get_config();
  if(!maybe_config) { return false; }

  // <folder>/<model>.txt
  std::string file_name = path + "/" + _name + ".txt";
  std::ofstream config_file(file_name);
  if(!config_file.is_open()) {
    throw std::runtime_error("Could not open config file for writing: " + file_name);
  }

  for (const auto& [key, value] : maybe_config.value()) {
    config_file << key << "=" << value << std::endl;
  }

  config_file.close();
  return true;
}

void Model::set_x0(Vec initial_state) 
{
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

bool Model::start_log(std::string& path) 
{
  std::string file_name = path + "/" + _name + ".csv";

  _log_file.open(file_name);
  if (!_log_file.is_open()) {
    throw std::runtime_error("Could not open log file: " + file_name);
  }
  csv_header(_log_file);
  return true;
}

void Model::csv_header(std::ostream& os) const
{
  os << "t";
  for(size_t i = 0; i < _n_states; i++) {
    os << "," << "x" << i;
  }
  os << std::endl;
}

void Model::csv_row(std::ostream& os) const 
{
  os << _t;
  for(size_t i = 0; i < _n_states; i++) {
    os << "," << _states[i];
  }
  os << std::endl;
}

