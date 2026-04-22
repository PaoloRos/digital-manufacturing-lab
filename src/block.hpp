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

// Machine will define at least: max_acc, max_deacc and time step dt
class Machine { 
public:
  Point zero() const { return Point(0.0, 0.0, 0.0); } // Reference point of the machine
                                                      // in Block _machine = const ptr -> zero must be a const method, otherwise it cannot be called from a const ptr
  data_t tq() const { return 0.01; }                  // Time step of the machine
  data_t A() const { return 1000.0; }                   // Maximum acceleration of the machine
};

// Block class represents a single block of G-code
// It's handy define it as a enum class, because in G-Code the commands are: G00->rapid, G01->line, G02->clockwise arc, G03->counterclockwise arc, etc...
class Block : public Object
{
public:
  
  // Type of block motion
  enum class BlockType {
    RAPID = 0,            // non-cutting rapid positioning
    LINE,                 // linear motion
    CWA,                  // clockwise arc
    CCWA,                 // counterclockwise arc
    NO_MOTION
  };

  // Motion profile: the main purpose of this CNC software is to calculate the motion profile in order to control the CNC machine.
  struct Profile {
    data_t a, d;                        // acceleration and deceleration
    data_t f, l;                        // feedrate and length of the block
    data_t fs, fe;                      // starting and final feedrate
    data_t dt_1, dt_m, dt_2;            // duration of the acceleration, cruise speed and deceleration phases
    data_t dt;                          // total duration
    data_t current_acc;                 // current acceleration along an arc
    data_t lambda(data_t t, data_t &s); // Motion interpolation: function lambda(t) = integral of velocity profile. Value in range [0,1]
                                        // It takes the time and the velocity as input, and return the value of lambda in range [0,1] and the current speed
  };

  // LIFECYCLE =================================================================
  
  Block(std::string line);
  Block(std::string line, Block &prev); // Constructor that takes the previous block as argument
  ~Block();
  std::string desc(bool colored = true) const override; // Return a description of the block
  Block &operator=(Block &o); // 'this' = 'other' as reference

  // OPERATIONS/OPERATORS ======================================================

  Block &parse(Machine const *m);                                 // Parse the line of G-code and extract the parameters
  data_t lambda(data_t time, data_t &speed);
  Point interpolate(data_t lambda);
  Point interpolate(data_t time, data_t &lambda, data_t &speed);   //[NOTE] I removed the reference to lambda in the arguments, since it can be calculated inside the function, avoiding the risk of out of scope reference.

  void walk(std::function<void(Block &b, data_t t, data_t l, data_t s)> func);  // Walk along the block, in steps of dt, executing lamdas function at every step along the trajectory: flessibilità di eseguire una funzione mentre avviene l'interpolazione

  // ACCESSORS =================================================================

  std::string line() const { return _line; }
  size_t n() const { return _n; }
  data_t dt() const { return _profile.dt; } // fake accessor
  BlockType type() const { return _type; }
  size_t tool() const { return _tool; }
  data_t feedrate() const { return _feedrate; }
  data_t arc_feedrate() const { return _arc_feedrate; }
  data_t spindle() const { return _spindle; }
  data_t length() const { return _length; }
  Point const &target() const { return _target; }
  Point const &center() const { return _center; }
  Point const &delta() const { return _delta; }
  size_t m() const { return _m; }
  Profile const &profile() const { return _profile; }
  bool parsed() const { return _parsed; }

  // My accessors
  data_t r() const { return _r; }
  data_t theta_0() const { return _theta_0; }
  data_t dtheta() const { return _dtheta; }

  Block *prev = nullptr;
  Block *next = nullptr;

private:
// --- Block ID ---
std::string _line;                        // original line of G-code, eg "N01 G00 X100"
size_t _n = 0;                            // block number
bool _parsed = false;                     // flag whther correctly parsed or not
  // --- Geometry ---
  Point _target = Point();                  // block destination, eg X100 Y100 Z0
  Point _center = Point();                  // to represent arc
  Point _delta = Point();                   // three projection (maybe the three components of the displacement vector)
  data_t _length = 0.0;                     // length of the displacement vector 
  data_t _i = 0.0, _j = 0.0, _r = 0.0;      // arc parameters: I and J for center, R for radius
  data_t _theta_0 = 0.0, _dtheta = 0.0;     // arc angles: initial and included angle
  // --- Machining ---
  data_t _feedrate = 0.0;
  data_t _arc_feedrate = 0.0;               // arc feedrate
  data_t _spindle = 0.0;                    // rotational spindle rate
  data_t _acc = 0.0;                        // actual acceleration
  size_t _m = 0.0;                          // M command argument, eg M03 -> _m = 3.0
  size_t _tool = 0.0;                       // tool number
  BlockType _type = BlockType::NO_MOTION;   // type of motion, eg rapid, line, arc, etc.
  Profile _profile;                         // motion profile, to be computed based on the block type and parameters
  Machine const *_machine = nullptr;        // pointer to the machine, without changing the Machine from the Block!

  bool parse_token(std::string const &token);
  Point start_point();
  void compute();
  void calc_arc();

};  // class Block

// Temporary


data_t Block::Profile::lambda(data_t t, data_t &s) { return 0.0; }

} // namespace cncpp

// ANNOTATIONS =================================================================

/*
Very first G-code command:
G00 X100 Y100 Z0: defining the first position of the tool --> positioning

in principle, the starting point is unknown (maybe the human operator has moved the tool...)

How to do? Ask to the machine (SW) where's the tool, in order to do the correct positioning. --> Implement in the Machine class



*/