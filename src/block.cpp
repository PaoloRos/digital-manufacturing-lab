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

using namespace std;
using namespace cncpp;

// ===== LIFECYCLE ==================================================================

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
  prev = &o;
  o.next = this;

  return *this;
}

// ====== OPERATIONS/OPERATORS ======================================================

Block &Block::parse(Machine const *m)
{
  _machine = m; // store the pointer to the machine, to access its parameters during the parsing and profile computation
  stringstream ss(_line);
  string token; // represents each single world at a time

  // _target empty
  while ( ss >> token ) {
    try{
      if (!parse_token(token)) break;
    } catch (runtime_error &e) {
      stringstream es;
      es << "Parsing error at line: " << _line << endl;
      es << " Token: " << token << endl;
      es << "Exception: " << e.what() << endl;
      throw runtime_error(es.str());
    }
  }
  // filling _target
  // Modal behaviour
  _target.modal(prev->target()); // perche non prev->_target?
  _delta = _target.delta(prev->target());
  //TODO: set _acc
  _length = _delta.length();

  // calculate profile
  switch (_type) 
  {
    case BlockType::LINE:
      // calculate line profile
      break;
    case BlockType::CWA:  // go in cascade to the next case
    case BlockType::CCWA:
      // calculate arc profile
      break;
    default:
      break;
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
Point Block::interpolate(data_t lambda)
{
  if (!_parsed)
    throw runtime_error("Cannot compute lambda: block not parsed!");
  Point result = Point();

  // Interpolation (linear)
  // x(t) = x(0) + d_x * lambda(t)
  // y(t) = y(0) + d_y * lambda(t)
  // z(t) = z(0) + d_z * lambda(t)
  
  if (_type == BlockType::LINE) {
    result.x(prev->target().x() + _delta.x() * lambda);
    result.y(prev->target().y() + _delta.y() * lambda);
  } else if (_type == BlockType::CWA || _type == BlockType::CCWA) {
    // circular interpolation: target = center + radius * [cos(theta), sin(theta)]
  } else { throw runtime_error("Cannot interpolate block " + _line); }
  
  result.z(prev->target().z() + _delta.z() * lambda); // is common

  return result;
}
Point Block::interpolate(data_t time, data_t &lambda, data_t &speed)
{
  if (!_parsed)
    throw runtime_error("Cannot compute lambda: block not parsed!");
}
void Block::walk(std::function<void(Block &b, data_t t, data_t l, data_t s)> func)
{

}

// ====== ACCESSORS ==================================================================




// ====== PRIVATE METHODS ============================================================

bool Block::parse_token(string const &token)
{
  bool res = true;


  return res;
}

// ====== TESTS ==================================================================

#ifdef BLOCK_TEST_MAIN

int main() {
  Block b1("G1 X10 Y20 Z30 F1000");

  cerr << (b1.parsed() ? "Block parsed" : "Block not parsed") << endl;

  Block b2("G1 X20 Y30 Z40 F1500", b1); // b2 inherits modal coords from b1

  return 0;
}

#endif // BLOCK_TEST_MAIN