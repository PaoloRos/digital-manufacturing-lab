/*
Block class implementation.

Implements a G-code block, which is a a collection of G-code commands that are executed together. A block can contain multiple commands, and each command can have multiple parameters.

Author: Paolo Rossi
Date: 2026-04-16
*/

#include "block.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <fmt/color.h>
#include <cmath>

using namespace std;
using namespace cncpp;

// LIFECYCLE ===================================================================

Block::Block(string line) : _line(line), _n(0) {
  cerr << fmt::format(fmt::fg(fmt::color::green) | fmt::emphasis::bold, "[Message]")
       << " Block " << _line << " created" << endl;
}

Block::Block(string line, Block &prev) : Block(line) { *this = prev; }

Block::~Block()
{
  cerr << fmt::format(fmt::fg(fmt::color::green) | fmt::emphasis::bold, "[Message]")
       << " Block " << _line << " destroyed" << endl;
}

string Block::desc(bool colored) const
{

}

Block &Block::operator=(Block &o)
{
  if ( !o._parsed )
    throw runtime_error("Cannot copy a Block that is not correctly parsed!");

  // inherite only modal coordinates and required parameters
  _tool = o._tool;
  _feedrate = o._feedrate;
  _spindle = o._spindle;
  _n = o._n + 1;            // increment block number
  _target.reset();          // reset target, to be updated by parse() if specified in the line
  // Linked list management
  prev = &o;
  o.next = this;

  return *this;
}

// OPERATIONS/OPERATORS ========================================================

Block &Block::parse(Machine const *m)
{
  _machine = m; // store the pointer to the machine, to access its parameters during the parsing and profile computation
  stringstream ss(_line);
  string token; // represents each single word at a time

  // _target empty
  while ( ss >> token ) {
    try{
      if (!parse_token(token)) break;
    } catch (runtime_error &e) {
      stringstream es;
      es << fmt::format(fmt::fg(fmt::color::red) | fmt::emphasis::bold, "[Error]");
      es << "Parsing error at line: " << _line << endl;
      es << " Token: " << token << endl;
      es << "Exception: " << e.what() << endl;
      throw runtime_error(es.str());
    }
  }
  // Filling _target:

  // Modal behaviour: this inherits the coordinates of the previous block

  // [QUESTION] There is the necessity to distinguish if the block is the first one of the chain: if yes, then it could be defined as the machine reference block.

  _target.modal(prev->target());
  _delta = _target.delta(prev->target());
  //TODO: set _acc
  _length = _delta.length();

  // Calculate profile
  switch (_type) 
  {
    case BlockType::LINE:
      // calculate line profile
      break;
    case BlockType::CWA:  // go in cascade to the next case
    case BlockType::CCWA:
      // calculate arc profile
      break;
    default: break;
  }

  _parsed = true;
  return *this;
}

data_t Block::lambda(data_t time, data_t &speed)
{
  if (!_parsed)
    throw runtime_error("Cannot compute lambda: block not parsed!");
  return _profile.lambda(time, speed);
}

// Implement the interpolation.
// There is a logical issue in some conditions
// Implement also the next one
// [Answer] What if *prev = nullptr?

Point Block::interpolate(data_t lambda)
{
  if (!_parsed)
    throw runtime_error("Cannot compute interpolation: block not parsed!");
  
    Point result = Point();
  
  // [See notes on Notability]
  if (_type == BlockType::LINE) {
    // Linear interpolation: target = start + lambda * delta
    result.x(prev->target().x() + _delta.x() * lambda);
    result.y(prev->target().y() + _delta.y() * lambda);
  } else if (_type == BlockType::CWA || _type == BlockType::CCWA) {
    // Circula interpolation:
    // [X,Y] = [Xc,Yc] + R * [cos(theta), sin(theta)]
    // [Z] = Z0 + lambda * delta_z
    // where theta = theta0 + lambda * delta_theta

    // Maybe necessary to distinguish between CCWA or CWA
  
    result.x( center().x() + r() * cos(prev->theta_0() +  dtheta() * lambda) );
    result.y( center().y() + r() * sin(prev->theta_0() +  dtheta() * lambda) );
  } else {
    throw runtime_error("Cannot interpolate block " + _line);
  }

  result.z(prev->target().z() + _delta.z() * lambda); // is common

  return result;
}

//[NOTE] I removed the reference to lambda in the arguments, since it can be calculated inside the function, avoiding the risk of out of scope reference.
Point Block::interpolate(data_t time, data_t lambda, data_t &speed)
{
  if (!_parsed)
    throw runtime_error("Cannot compute lambda: block not parsed!");
  return interpolate( Block::lambda(time, speed) );
}


void Block::walk(std::function<void(Block &b, data_t t, data_t l, data_t s)> func)
{

}

// ACCESSORS ===================================================================




// PRIVATE METHODS =============================================================

bool Block::parse_token(string const &token)
{
  bool res = true;


  return res;
}

// TESTS =======================================================================

#ifdef BLOCK_TEST_MAIN

int main() {
  Block b1("G1 X10 Y20 Z30 F1000");

  cerr << (b1.parsed() ? "Block parsed" : "Block not parsed") << endl;

  Block b2("G1 X20 Y30 Z40 F1500", b1); // b2 inherits modal coords from b1

  return 0;
}

#endif // BLOCK_TEST_MAIN