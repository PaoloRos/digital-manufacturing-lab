/*
 ____  _            _           _               
| __ )| | ___   ___| | __   ___| | __ _ ___ ___ 
|  _ \| |/ _ \ / __| |/ /  / __| |/ _` / __/ __|
| |_) | | (_) | (__|   <  | (__| | (_| \__ \__ \
|____/|_|\___/ \___|_|\_\  \___|_|\__,_|___/___/
         

Implements a G-code block, which is a a collection of G-code commands that are executed together. A block can contain multiple commands, and each command can have multiple parameters.

Author_ Paolo Rossi
Date: 2026-04-16
*/

#pragma once

#include "defines.hpp"
#include "point.hpp"

#include <string>
#include <functional>

namespace cncpp {

/**
 * @brief Minimal machine interface used by Block during parsing/interpolation.
 */
class Machine { 
public:
  /**
   * @brief Get the machine reference point.
   * @return Machine origin point.
   */
  Point zero() const { return Point(0.0, 0.0, 0.0); } // Reference point of the machine.
  /**
   * @brief Get machine control period.
   * @return Time quantum in seconds.
   */
  data_t tq() const { return 0.01; }                  // Time step of the machine.
  /**
   * @brief Get machine maximum acceleration.
   * @return Maximum acceleration value.
   */
  data_t A() const { return 1000.0; }                 // Maximum acceleration of the machine
};

/**
 * @brief Representation of a single G-code block.
 *
 * A block stores parsed command parameters and provides trajectory utilities
 * such as profile generation and interpolation.
 */
class Block : public Object
{
public:
  /**
   * @brief Motion type encoded by G commands.
   */
  enum class BlockType {
    RAPID = 0,            // non-cutting rapid positioning
    LINE,                 // linear motion
    CWA,                  // clockwise arc
    CCWA,                 // counterclockwise arc
    NO_MOTION
  };

  /**
   * @brief Kinematic profile associated with the block.
   */
  struct Profile {
    data_t a = 0.0, d = 0.0;            // acceleration and deceleration
    data_t f = 0.0, l = 0.0;            // feedrate and length of the block
    data_t fs = 0.0, fe = 0.0;          // starting and final feedrate
    data_t dt_1 = 0.0, dt_m = 0.0, dt_2 = 0.0;  // duration of acceleration, cruise and deceleration phases
    data_t dt = 0.0;                    // total duration
    data_t current_acc = 0.0;           // current acceleration along an arc
    /**
     * @brief Compute normalized trajectory progress at time t.
     * @param t Time from block start.
     * @param s Output current speed.
     * @return Progress factor in range [0, 1].
     */
    data_t lambda(data_t t, data_t &s); // Motion interpolation: function lambda(t) = integral of velocity profile. It takes the time and the velocity as input, and return the value of lambda in range [0,1] and the current speed
  };

  // LIFECYCLE =================================================================
  
  /**
   * @brief Construct a block from a raw G-code line.
   * @param line Raw block text.
   */
  Block(std::string line);
  /**
   * @brief Construct a block inheriting modal values from previous block.
   * @param line Raw block text.
   * @param prev Previous parsed block.
   */
  Block(std::string line, Block &prev); // Constructor that takes the previous block as argument
  /** @brief Destroy the block. */
  ~Block();
  /**
   * @brief Build a string description of the block.
   * @param colored Enable ANSI colored output when true.
   * @return Human-readable block summary.
   */
  std::string desc(bool colored = true) const override; // Returns a description of the block
  /**
   * @brief Copy modal state from another parsed block.
   * @param o Source block.
   * @return Reference to this block.
   */
  Block &operator=(Block &o); // 'this' = 'other' as reference

  // OPERATIONS/OPERATORS ======================================================

  /**
   * @brief Parse block tokens using machine constraints.
   * @param m Pointer to machine context.
   * @return Reference to this block.
   */
  Block &parse(Machine const *m);                                 // Parse the line of G-code and extract the parameters
  /**
   * @brief Evaluate profile progress for a given time.
   * @param time Elapsed block time.
   * @param speed Output current speed.
   * @return Progress factor in range [0, 1].
   */
  data_t lambda(data_t time, data_t &speed);
  /**
   * @brief Interpolate block pose from normalized progress.
   * @param lambda Progress factor in range [0, 1].
   * @return Interpolated point.
   */
  Point interpolate(data_t lambda);
  /**
   * @brief Interpolate block pose from time.
   * @param time Elapsed block time.
   * @param lambda Output normalized progress.
   * @param speed Output current speed.
   * @return Interpolated point.
   */
  Point interpolate(data_t time, data_t &lambda, data_t &speed);

  /**
   * @brief Iterate along the block with machine time step.
   * @param func Callback receiving block reference, time, lambda and speed.
   */
  void walk(std::function<void(Block &b, data_t t, data_t l, data_t s)> func);  // Walk along the block, in steps of dt, executing lamdas function at every step along the trajectory: flessibilità di eseguire una funzione mentre avviene l'interpolazione

  // ACCESSORS =================================================================

  /** @brief Get original G-code line. */
  std::string line() const { return _line; }
  /** @brief Get block sequence number. */
  size_t n() const { return _n; }
  /** @brief Get total profile duration. */
  data_t dt() const { return _profile.dt; } // fake accessor
  /** @brief Get block motion type. */
  BlockType type() const { return _type; }
  /** @brief Get selected tool number. */
  size_t tool() const { return _tool; }
  /** @brief Get nominal feedrate. */
  data_t feedrate() const { return _feedrate; }
  /** @brief Get feedrate limited for arc motion. */
  data_t arc_feedrate() const { return _arc_feedrate; }
  /** @brief Get spindle speed. */
  data_t spindle() const { return _spindle; }
  /** @brief Get geometric path length. */
  data_t length() const { return _length; }
  /** @brief Get target point. */
  Point const &target() const { return _target; }
  /** @brief Get arc center point. */
  Point const &center() const { return _center; }
  /** @brief Get displacement vector. */
  Point const &delta() const { return _delta; }
  /** @brief Get M command argument. */
  size_t m() const { return _m; }
  /** @brief Get kinematic profile. */
  Profile const &profile() const { return _profile; }
  /** @brief Check whether block has been successfully parsed. */
  bool parsed() const { return _parsed; }

  // My accessors
  /** @brief Get arc radius. */
  data_t r() const { return _r; }
  /** @brief Get initial arc angle. */
  data_t theta_0() const { return _theta_0; }
  /** @brief Get arc swept angle. */
  data_t dtheta() const { return _dtheta; }

  /** @brief Previous block in linked sequence (nullable). */
  Block *prev = nullptr;
  /** @brief Next block in linked sequence (nullable). */
  Block *next = nullptr;

private:
// --- Block ID ---
std::string _line;                            // original line of G-code, eg "N01 G00 X100"
size_t _n = 0;                                // block number
bool _parsed = false;                         // flag whther correctly parsed or not
  // --- Geometry ---
  Point _target = Point();                    // block destination, eg X100 Y100 Z0
  Point _center = Point();                    // to represent arc
  Point _delta = Point();                     // three projection (maybe the three components of the displacement vector)
  data_t _length = 0.0;                       // length of the displacement vector 
  data_t _i = 0.0, _j = 0.0, _r = 0.0;        // arc parameters: I and J for center, R for radius
  data_t _theta_0 = 0.0, _dtheta = 0.0;       // arc angles: initial and included angle
  // --- Machining ---
  data_t _feedrate = 0.0;
  data_t _arc_feedrate = 0.0;                 // arc feedrate
  data_t _spindle = 0.0;                      // rotational spindle rate
  data_t _acc = 0.0;                          // actual acceleration
  size_t _m = 0.0;                            // M command argument, eg M03 -> _m = 3.0
  size_t _tool = 0.0;                         // tool number
  BlockType _type = BlockType::NO_MOTION;     // type of motion, eg rapid, line, arc, etc.
  Profile _profile;                           // motion profile, to be computed based on the block type and parameters
  Machine const *_machine = nullptr;          // Pointer to a read-only machine (const machine: machine mustn't be changed by a generic g-code block).

  /** @brief Parse one token from the G-code line. */
  bool parse_token(std::string const &token); // Parse the &token, returning true if successful
  /** @brief Resolve interpolation start point from previous block or machine zero. */
  Point start_point() const; // Starting point of the block: if there is a previous block, it's the target of the previous block, otherwise it's the machine reference point
  /** @brief Compute motion profile parameters. */
  void compute();
  /** @brief Compute arc geometric parameters. */
  void calc_arc();

};  // class Block

// INLINE ======================================================================

inline Point Block::start_point() const 
{
  if(!_machine)
    throw std::logic_error("Machine not set for block " + std::to_string(_n));

  return prev ? prev->target() : _machine->zero();
}

// Temporary

inline data_t Block::Profile::lambda(data_t t, data_t &s)
{
  (void)t;
  s = 0.0;
  return 0.0;
}

} // namespace cncpp

// ANNOTATIONS =================================================================

/*
Very first G-code command:
G00 X100 Y100 Z0: defining the first position of the tool --> positioning

In principle, the starting point is unknown (maybe the human operator has moved the tool...)

How to do? Ask to the machine (SW) where's the tool, in order to do the correct positioning --> Implement in the Machine class a zero() mathod that returns the reference point of the machine, which is the starting point of the first block. 
*/