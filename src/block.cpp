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
  //cerr << log_tag(LogType::MESSAGE, cerr)
  //     << " Block " << _line << " created" << endl;
}

Block::Block(string line, Block &prev) : Block(line) { *this = prev; }

Block::~Block()
{
  //cerr << log_tag(LogType::MESSAGE, cerr)
  //     << " Block " << _line << " destroyed" << endl;
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

      std::cerr << log_tag(LogType::ERROR, std::cerr) << ' ' << msg << std::endl;
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
  cerr << log_tag(LogType::COMPUTATION, cerr) << ' '
       << "Computing motion profile for block " << _line << endl;

  data_t const &l = _length, &A = _acc;
  data_t dt, dt_1, dt_m, dt_2, dq;
  data_t f_m;
  data_t a, d;

  f_m = _arc_feedrate / 60.0;
  dt_1 = f_m / A;
  dt_2 = dt_1;
  dt_m = l / f_m - (dt_1 + dt_2) / 2.0;

  if (dt_m > 0) {                                     // long block, trapezoid
    dt = _machine->quantize(dt_1 + dt_m + dt_2, dq);
    dt_m = dt_m + dq;
    f_m = (2 * l) / (dt_1 + dt_2 + 2 * dt_m);
  } else {                                            // short block, triangle
    dt_1 = dt_2 = sqrt(l / A);
    dt = _machine->quantize(dt_1 + dt_2, dq);
    dt_m = 0;
    dt_2 = dt_2 + dq;
    f_m = 2 * l / (dt_1 + dt_2);
  }
  a = f_m / dt_1;
  d = -(f_m / dt_2);
  _profile.dt_1 = dt_1;
  _profile.dt_2 = dt_2;
  _profile.dt_m = dt_m;
  _profile.a = a;
  _profile.d = d;
  _profile.f = f_m;
  _profile.dt = dt;
  _profile.l = l;
}

void Block::calc_arc()
{
  data_t x0, y0, z0, xc, yc, xf, yf, zf;
  Point p0 = start_point();
  x0 = p0.x();
  y0 = p0.y();
  z0 = p0.z();
  xf = _target.x();
  yf = _target.y();
  zf = _target.z();

  if (_r) { // if the radius is given
    data_t dx = _delta.x();
    data_t dy = _delta.y();
    // signs table
    // sign(r) | CW(-1) | CCW(+1)
    // --------------------------
    //      -1 |     +  |    -
    //      +1 |     -  |    +
    int s = (_r > 0) - (_r < 0);
    s *= (_type == BlockType::CCWA ? 1 : -1);
    data_t d = hypot(dx, dy);
    data_t sq = sqrt(pow(_r, 2) - pow(d, 2)/4.0) / d;
    xc = (x0 + xf)/2.0 - s * dy * sq;
    yc = (y0 + yf)/2.0 + s * dx * sq;
  } else { // if I,J are given
    data_t r2;
    _r = hypot(_i, _j);
    xc = x0 + _i;
    yc = y0 + _j;
    r2 = hypot(xf - xc, yf - yc);
    if (fabs(_r - r2) > _machine->error()) {
      throw runtime_error(
          fmt::format("Arc endpoints mismatch error ({:})", _r - r2).c_str());
    }
  }
  _center.x(xc);
  _center.y(yc);
  _theta_0 = atan2(y0 - yc, x0 - xc);
  _dtheta = atan2(yf - yc, xf - xc) - _theta_0;
  // we need the net angle so we take the 2PI complement if negative
  if (_dtheta < 0)
    _dtheta = 2 * M_PI + _dtheta;
  // if CW, take the negative complement
  if (_type == BlockType::CWA)
    _dtheta = -(2 * M_PI - _dtheta);
  //
  _length = fabs(_dtheta * _r);
  // from now on, it's safer to drop the sign of radius angle
  _r = fabs(_r);
}

/*
 ____             __ _ _            _                   _   
|  _ \ _ __ ___  / _(_) | ___   ___| |_ _ __ _   _  ___| |_ 
| |_) | '__/ _ \| |_| | |/ _ \ / __| __| '__| | | |/ __| __|
|  __/| | | (_) |  _| | |  __/ \__ \ |_| |  | |_| | (__| |_ 
|_|   |_|  \___/|_| |_|_|\___| |___/\__|_|   \__,_|\___|\__|
                                                           
*/

data_t Block::Profile::lambda(data_t t, data_t &s)
{
  data_t r;           // walked distance along the block at time t
  current_acc = 0.0;

  if (t < 0) {
    r = 0.0;
    s = 0.0;
  } else if (t < dt_1) {                // acceleration phase
    r = a * t*t / 2.0;
    s = a * t;
    current_acc = a;
  } else if (t < dt_1 + dt_m) {         // cruise phase
    r = f * (dt_1 / 2.0 + (t - dt_1));
    s = f;
    current_acc = 0;
  } else if (t < dt_1 + dt_m + dt_2) {  // deceleration phase
    data_t t_2 = dt_1 + dt_m;
    r = f * (dt_1 / 2.0 + dt_m ) + f * (t - t_2) + 
        d / 2.0 * (t*t + t_2*t_2) - d * t * t_2;
    s = f + d * (t - t_2);
    current_acc = d;
  } else {
    r = l;
    s = 0.0;
  }

  r /= l;             // normalize the walked distance
  s *= 60;            // convert speed from mm/s to mm/min 
  return r;
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