/*
 __  __            _     _               ____ _               
|  \/  | __ _  ___| |__ (_)_ __   ___   / ___| | __ _ ___ ___ 
| |\/| |/ _` |/ __| '_ \| | '_ \ / _ \ | |   | |/ _` / __/ __|
| |  | | (_| | (__| | | | | | | |  __/ | |___| | (_| \__ \__ \
|_|  |_|\__,_|\___|_| |_|_|_| |_|\___|  \____|_|\__,_|___/___/
                                                             

Machine definition and interface.

Author: Paolo Rossi
Date: 2026-04-25
*/

#pragma once
#include "defines.hpp"
#include "point.hpp"
#include <nlohmann/json.hpp>
#include <toml++/toml.hpp>
#include <agent.hpp>

namespace cncpp {

/**
 * @brief Minimal machine interface used by Block during parsing/interpolation.
 */
class Machine : public Object { 
  public:
  // LIFECYCLE =================================================================
  
  /** @brief Create a machine with default parameters. */
  Machine() = default;

  /**
   * @brief Create a machine from JSON data.
   * @param j JSON document containing machine configuration.
   */
  Machine(nlohmann::json &j);

  /**
   * @brief Create a machine from a JSON file.
   * @param filename Path to the JSON configuration file.
   */
  Machine(std::string &filename);

  /** @brief Destroy the machine instance. */
  ~Machine();

  /**
   * @brief Build a string description of the machine.
   * @param colored Enable ANSI colored output when true.
   * @return Human-readable machine summary.
   */
  std::string desc(bool colored = true) const override;

  // OPERATIONS/OPERATORS ======================================================
  
  /**
   * @brief Load machine configuration from JSON data.
   * @param j JSON document containing machine configuration.
   */
  void load(nlohmann::json &j);

  /**
   * @brief Load machine configuration from TOML data.
   * @param t TOML table containing machine configuration.
   */
  void load(toml::table &t);

  /**
   * @brief Load machine configuration from a JSON file.
   * @param filename Path to the JSON configuration file.
   */
  void load(std::string &filename);

  /**
   * @brief Quantize a time value to the next control tick.
   * @param t Input time value.
   * @param dq Output quantization error, computed as @f$q - t@f$.
   * @return Next quantized time @f$q@f$, with @f$q > t@f$.
   */
  data_t quantize(data_t t, data_t &dq) const;

  // MADS RELATED ==============================================================

  // Connection to the Network
  // Default MAds URL is "tcp://localhost:9092"
  void connect(std::string const &name, std::string const &url = "tcp://localhost:9092");

  // Synchronizing the current state to the remote state (the FMU simulator)
  void sync();

  // Sending a setpoint for the axes
  void set_setpoint(Point const &p);

  void reset();

  // Knowing whether the client is connected to the server
  bool is_connected() const { return _agent && _agent->is_connected(); }

  
  
  // ACCESSORS =================================================================
  /**
   * @brief Get machine maximum acceleration.
   * @return Maximum acceleration value.
   */
  data_t A() const { return _A; }
  
  /**
   * @brief Get machine control period.
   * @return Quantization period in seconds.
   */
  data_t tq() const { return _tq; }
  
  /**
   * @brief Gets the max machine sampling period.
   * @return Quantization period in seconds.
   */
  data_t tq_max() const { return _tq_max; }
  
  /**
   * @brief Get machine maximum feedrate.
   * @return Maximum feedrate value.
   */
  data_t fmax() const { return _fmax; }
  
  /**
   * @brief Get machine maximum positioning error.
   * @return Maximum allowed error in mm.
   */
  data_t max_error() const { return _max_error; }
  
  /**
   * @brief Get current positioning error.
   * @return Current error in mm.
   */
  data_t error() const { return _error; }
  
  /**
   * @brief Get the machine reference point.
   * @return Machine origin point.
   */
  Point zero() const { return _zero; }
  
  /**
   * @brief Get the machine offset from the reference point.
   * @return Offset point.
   */
  Point offset() const { return _offset; }
  
  /**
   * @brief Get current machine setpoint.
   * @return Setpoint coordinates.
   */
  Point setpoint() const { return _setpoint; }
  
  /**
   * @brief Set machine setpoint.
   * @param p New setpoint.
   * @return Updated setpoint.
   */
  Point setpoint(Point p) { return _setpoint = p; }
  
  /**
   * @brief Set machine setpoint.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param z Z coordinate.
   * @return Updated setpoint.
   */
  Point setpoint(data_t x, data_t y, data_t z) { return _setpoint = Point(x, y, z); }
  
  /**
   * @brief Get current machine position.
   * @return Position coordinates.
   */
  Point position() const { return _position; }
  
  /**
   * @brief Update machine position.
   * @param p New position.
   * @return Updated position.
   */
  Point position(Point p) { return _position = p; }
  
  /**
   * @brief Get the raw configuration data as JSON.
   * @return Stored JSON configuration data.
   */
  nlohmann::json data() const { return _data; }
  
  // Return a pointer to the MADS agent
  Mads::Agent *agent() const { return _agent.get(); }
  
  nlohmann::json state() const { return _state; }
  
  private:
  nlohmann::json _data{};
  
  // Properties
  Point _zero{0.0, 0.0, 0.0};   // machine reference point (origin)
  Point _offset{0.0, 0.0, 0.0}; // offset of the workpiece origin from the machine reference point
  data_t _tq = 0.001;           // (ms) control period in seconds (quantization step for time)
  data_t _tq_max = 0.01;        // maximum allowed control period in secs (for safety)
  data_t _A = 5.0;              // maximum acceleration in mm/s^2
  data_t _fmax = 10000.0;       // maximum feedrate in mm/min
  data_t _max_error = 0.005;    // maximum allowed positioning error in mm
  
  // State parameters
  Point _setpoint, _position;   // current setpoint and actual position
  data_t _error = numeric_limits<data_t>::quiet_NaN();          // current positioning error in mm
  // quiet_NaN() is used to indicate that the error is not yet computed/available, without throwing an exception in runtime (due to NaN)
  
  // MADS agent
  
  void clear_command();
  
  std::unique_ptr<Mads::Agent> _agent;
  nlohmann::json _command{};    // json encoding the setpoint to be sent to the simulator driver
  nlohmann::json _state{};      // json encoding the current machine state received from the simulator driver
  
  
  
  
};  // class Machine

};  // namespace cncpp