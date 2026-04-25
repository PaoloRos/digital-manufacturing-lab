/*
Block class implementation.

Implements a G-code block, which is a a collection of G-code commands that are executed together. A block can contain multiple commands, and each command can have multiple parameters.

Author: Paolo Rossi
Date: 2026-04-16
*/

#include "block.hpp"

#include <string>
#include <sstream>
#include <fmt/color.h>
#include <fmt/format.h>
#include <cmath>
#include <ctype.h>

using namespace std;
using namespace fmt;

using namespace cncpp;

// LIFECYCLE ===================================================================

Block::Block(string line) : _line(line), _n(0) {
  cerr << log_tag(LogType::Message, cerr)
       << " Block " << _line << " created" << endl;
}

Block::Block(string line, Block &prev) : Block(line) { *this = prev; }

Block::~Block()
{
  cerr << log_tag(LogType::Message, cerr)
       << " Block " << _line << " destroyed" << endl;
}

string Block::desc(bool colored) const
{
  if (!_parsed)
    throw runtime_error("Block not parsed yet: cannot describe!");

  stringstream ss;
  
  // Default block type color: LINE, CWA AND CCWA
  auto block_color = color::green;

  if (_type == BlockType::NO_MOTION) { block_color = color::gray; }
  else if (_type == BlockType::RAPID) { block_color = color::red; }

  ss << format("[{:>3}] ", _n);
  if (colored)
    ss << format("G{:0>2} ", styled(static_cast<int>(_type), fg(block_color)));
  else
    ss << format("G{:0>2} ", static_cast<int>(_type));

  ss << _target.desc(colored);
  ss << format(" F{:>5.0f} S{:>4.0f} ", _feedrate, _spindle);
  ss << format("T{:0>2} M{:0>2}", _tool, _m);
  ss << format(" L{:>6.2f}mm DT{:>6.2f}s", _length, _profile.dt);

  return ss.str();
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
  _machine = m;           // store the pointer to the machine, to access (read-only) its parameters during the parsing and profile computation
  stringstream ss(_line);
  string token;           // represents each single word at a time

  // _target =  empty
  while ( ss >> token ) {
    try{
      if (!parse_token(token)) break;
    } catch (std::exception const &e) {
      auto const msg = fmt::format(
        "Parsing error at line: '{}'. Token: '{}': '{}",
         _line, token, e.what() );

      std::cerr << log_tag(LogType::Error, std::cerr) << ' ' << msg << std::endl;
      throw runtime_error(msg);
    }
  }

  // Filling _target:
  // Modal behaviour: this inherits the coordinates of the previous block
  _target.modal(start_point());
  _delta = _target.delta(start_point());
  _acc = _machine->A();
  _length = _delta.length();

  // Calculate profile
  switch (_type) 
  {
    case BlockType::LINE:
      // calculate line profile
      _arc_feedrate = _feedrate; // arc feedrate = nominal one
      compute();
      break;

    case BlockType::CWA:        // go in cascade to the next case
    case BlockType::CCWA:
      // calculate arc profile
      calc_arc();
      // arc feedrate is the minimum between the nominal feedrate and the 
      // maximum feedrate allowed by the machine acceleration along an arc of 
      // radius r: v^2 = a^2 * r^2 in seconds
      _arc_feedrate = min(
        _feedrate,
        pow( _machine->A()*_machine->A() * _r*_r, 0.25 ) * 60 
      );
      compute();
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

Point Block::interpolate(data_t lambda)
{
  if (!_parsed)
    throw runtime_error("Cannot compute interpolation: block not parsed!");
  
  Point result = Point();
  Point p0 = start_point();
  
  // [See notes on Notability]
  // Block is a straight line
  if (_type == BlockType::LINE) {
    // Linear interpolation: target = start + lambda * delta
    result.x(p0.x() + _delta.x() * lambda);
    result.y(p0.y() + _delta.y() * lambda);
  } 
  // Block is an arc
  else if (_type == BlockType::CWA || _type == BlockType::CCWA) {
    // Circular interpolation:
    // [X,Y] = [Xc,Yc] + R * [cos(theta), sin(theta)]
    // [Z] = Z0 + lambda * delta_z
    // where theta = theta0 + lambda * delta_theta

    data_t angle = _theta_0 + _dtheta * lambda; // current angle
    result.x( _center.x() + _r * cos(angle) );
    result.y( _center.y() + _r * sin(angle) );
  } 
  // Something wrong
  else {
    throw runtime_error("Cannot interpolate block " + _line);
  }

  result.z(p0.z() + _delta.z() * lambda); // is common

  return result;
}

Point Block::interpolate(data_t time, data_t &lambda, data_t &speed)
{
  if (!_parsed)
    throw runtime_error("Cannot compute lambda: block not parsed!");

  lambda = this->lambda(time, speed);
  return interpolate(lambda);
}


// How to call walk:
// b.walk(
//     [&](Block &b, data_t t, data_t l, data_t s
// ) { 
      /* do something with b, t, l and s */
// } );
// Walk walks through the entire block


void Block::walk(std::function<void(Block &b, data_t t, data_t l, data_t s)> func)
{
  data_t t = 0.0, l, s;
  while (t <= _profile.dt) {    // smaller than block duration
    l = lambda(t, s);           // update lambda and speed
    func( *this, t, l, s);      // execute the function with the current block, time, lambda and speed
    t += _machine->tq();        // increment time by machine time step
  }
}

// ACCESSORS ===================================================================




// PRIVATE METHODS =============================================================

bool Block::parse_token(string const &token)
{
  bool res = true;
  char cmd = toupper(token[0]);     // first character of the token, representing the command. User-friendly version: accept token in upper or lower case
  string arg = token.substr(1);     // the rest (substr() ) of the token, representing the argument of the command

  if ( cmd == '#' || cmd == ';' )   // comments
    return false;                   // not a valid command
  if (arg.empty())                  // empty argument or space --> error
    throw invalid_argument("Missing argument for command " + string(1, cmd));

  // Everything okay
  switch (cmd)
  {
    case 'N':
      _n = stoi(arg);
      if (prev && prev->_n >= _n)
        throw invalid_argument(
          "Block number " + to_string(format("[{:>3}] ", arg)) + " is NOT greater than previous block number " 
          + to_string(format("[{:>3}] ", prev->_n))
        );
    
    case 'G':
      _type = static_cast<BlockType>(stoi(arg));
      break;

    case 'X':
      _target.x(stod(arg));
      break;
    
    case 'Y':
      _target.y(stod(arg));
      break;
    
    case 'Z':
      _target.z(stod(arg));
      break;

    case 'I':
      _i = stod(arg);
      break;

    case 'J':
      _j = stod(arg);
      break;

    case 'R':
      _r = stod(arg);
      break;

    case 'F':
      _feedrate = stod(arg);
      break;

    case 'S':
      _spindle = stod(arg);
      break;

    case 'T':
      _tool = stoi(arg);
      break;

    case 'M':
      _m = stoi(arg);
      break;

    default:
      throw invalid_argument("Unknown command " + string(1, cmd));
  }

  return res;
}


void Block::compute()
{
  cerr << log_tag(LogType::Computation, cerr) << ' '
       << "Computing motion profile for block " << _line << endl;
}

void Block::calc_arc()
{
  cerr << log_tag(LogType::Computation, cerr) << ' '
       << "Calculating arc parameters for block " << _line << endl;
}

/*
 _____         _     __  __       _       
|_   _|__  ___| |_  |  \/  | __ _(_)_ __  
  | |/ _ \/ __| __| | |\/| |/ _` | | '_ \ 
  | |  __/\__ \ |_  | |  | | (_| | | | | |
  |_|\___||___/\__| |_|  |_|\__,_|_|_| |_|
                                          
*/

#ifdef CNCPP_BLOCK_TEST_MAIN
#include <iostream>

using namespace std;

int main() {

  Machine m{};

  Block b1{"N01 G00 X100 Y100 z200"};
  Block b2{"N02 G00 Z150", b1.parse(&m)};
  Block b3{"n03 G01 x50 y20 T1 f5000 s200 M3", b2.parse(&m)};
  b3.parse(&m); 
  cerr << b1 << endl
       << b2 << endl
       << b3 << endl;

  // walk along b3 and routinely print the time, coordinates, and feedrate
  // first: print a header line
  cout << "t,lambda,s,x,y,z" << endl;
  b3.walk([&](Block &b, data_t t, data_t l, data_t s){
    Point pos = b.interpolate(l);
    cout << format("{:},{:},{:},{:},{:},{:}", t, l, s, pos.x(), pos.y(), pos.z()) << endl;
  });

  return 0;
}

#endif // CNCPP_BLOCK_TEST_MAIN