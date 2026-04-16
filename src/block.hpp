// Block of general functionality, that can be reused across the application and together.

/*
Block class

Implements a G-code block, which is a a collection of G-code commands that are executed together. A block can contain multiple commands, and each command can have multiple parameters.
*/

#pragma once

#include "defines.hpp"
#include "point.hpp"

#include <string>
#include <functional>

namespace cncpp {

// Machine will define at least: max_acc, max_deacc and time step dt

class Machine 
{

};

class Block
{
public:
  
  enum class BlockType {
    RAPID = 0,            // force the index of the enum (to avoid differnt compiler behavior) and handy to read
    LINE,
    CWA,                  // clockwise arc
    CCWA,                 // counterclockwise arc
    NO_MOTION
  };

  struct Profile {
    data_t a, d;  // acceleration and deceleration
    data_t f, l;
    data_t fs, fe;
    data_t dt_1, dt_m, dt_2;
    data_t dt;  // duration
    data_t current_acc;
    data_t lambda(data_t t, data_t &s); // return a value in range [0,1] and the current speed
  };
  // LYFECYCLE
  Block(std::string line);
  Block(std::string line, Block &prev); // constructor that takes the previous block as argument, to compute the modal coordinates and other parameters based on the previous block
  ~Block();
  std::string desc(bool colored = true) const; // return a description of the block, with optional colored output (using fmt library)
  Block &operator=(Block &o); // 'this' = 'other' as reference

  // OPERATIONS/OPERATORS

  Block &parse(Machine const *m); // parse the line of G-code and extract the parameters
  data_t lambda(data_t time, data_t &speed);
  Point interpolate(data_t lambda);
  Point interpolate(data_t time, data_t &lambda, data_t &speed);  // in one shot: lamda and speed returned
  void walk(std::function<void(Block &b, data_t t, data_t l, data_t s)> func);  // walk along the block, in steps of dt, executing lamdas function at every step along the trajectory: flessibilità di eseguire una funzione mentre avviene l'interpolazione

  // ACCESSORS
  std::string line() const { return _line; }
  size_t n() const { return _n; }
  data_t dt() const { return _profile.dt; } // fake accessor
  BlockType type() const { return _type; }
  size_t tool() const { return _tool; }
  data_t feedrate() const { return _feedrate; }
  data_t arc_feedrate() const { return _arc_feedrate; }
  data_t spindle() const { return _spindle; }
  data_t length() const { return _length; }
  const Point &target() const { return _target; }
  const Point &center() const { return _center; }
  const Point &delta() const { return _delta; }
  size_t m() const { return _m; }
  const Profile &profile() const { return _profile; }
  bool parsed() const { return _parsed; }

  // public because need to be accessible
  Block *prev = nullptr;
  Block *next = nullptr;
  

private:
  std::string _line;                        // original line of G-code, eg "N01 G00 X100"
  size_t _n = 0;                            // block number
  // Geometry
  Point _target = Point();
  Point _center = Point();                  // to represent arc
  Point _delta = Point();                   // tree projection
  data_t _length = 0.0;
  data_t _i = 0.0, _j = 0.0, _r = 0.0;      // arc parameters
  data_t _theta_0 = 0.0, _dtheta = 0.0;     // arc angles
  // Machining
  data_t _feedrate = 0.0;
  data_t _arc_feedrate = 0.0;               // feedrate along the arc
  data_t _spindle = 0.0;                    // spindle rate
  data_t _acc = 0.0;                        // actual accelaration
  size_t _m = 0.0;                          // M command argument, eg M03 -> _m = 3.0
  size_t _tool = 0.0;                      // tool number
  BlockType _type = BlockType::NO_MOTION;   // type of motion, eg rapid, line, arc, etc.
  Profile _profile;                         // motion profile, to be computed based on the block type and parameters
  Machine const *_machine = nullptr;        // pointer to the machine, without changine the machine frome the Block!
  bool _parsed = false;                    // flag to indicate if the block has been parsed correctly, to avoid copying unparsed blocks in the assignment operator

bool parse_token(string const &token);

};  // class Block


} // namespace cncpp

// ========= ANNOTATIONS ========

// in sructs all members are public by default: no underscore at the beginning of the name, to avoid confusion with private members of classes, that are usually prefixed with an underscore. Moreover, since the struct is used as a simple data container, it doesn't need the encapsulation provided by private members and accessors, so we can keep all members public for simplicity and readability.