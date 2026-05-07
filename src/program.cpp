/*
Program class implementation.

Author: Paolo Rossi
Date: 2026-04-25
*/

#include "program.hpp"
#include "defines.hpp"
#include <fstream>

using namespace std;
using namespace cncpp;

// LIFECYCLE ===================================================================

Program::Program(string const &f, Machine *m) : _m(m), _filename(f)
{
  load(_filename);

  //cerr << log_tag(LogType::MESSAGE, cerr)
  //     << " Program " << _filename << " created" << endl;
} 
  
Program::Program(Machine *m) : _m(m)
{
  //cerr << log_tag(LogType::MESSAGE, cerr)
  //     << " Program created with no source file" << endl;
}

Program::~Program()
{
  //cerr << log_tag(LogType::MESSAGE, cerr)
  //     << " Program " << _filename << " destroyed" << endl;
}

string Program::desc(bool colored) const 
{
  // List of blocks -> iterate the call Block.desc for each block
  ostringstream ss;
  for (auto &current_block : *this)
    ss << current_block.desc(colored) << endl;
  return ss.str();
}

// OPERATIONS/OPERATORS ========================================================

void Program::load(string const &f, bool append)
{
  cerr << log_tag(LogType::MESSAGE, cerr)
  << " Loading program from file " << f << "..." << endl;
  
  _filename = f;
  
  ifstream file(_filename);
  if (!file.is_open())
    throw runtime_error("Could not open file " + _filename);
  
  if(!append) reset();
  
  string line;
  while (getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;
    *this << line;
  }
  file.close();
}

Program &Program::operator<<(string const &line)
{
  if (this->size() > 0)                     // not the first block
    this->emplace_back(line, this->back()); // create a new object at the end of the list, using the constructor with modal inheritance
  else                                      // first block
    this->emplace_back(line);               // create a new object at the end of the list, using the constructor WITHOUT modal inheritance
  
  this->back().parse(_m);
  return *this;
}

block_iterator Program::load_next() 
{
  if (_current == this->end()) 
    _current = this->begin();
  else
    _current++;

  _done = (_current == this->end());
  return _current;
}

void Program::rewind()
{
  _current = this->end();
  _done = false;
}

void Program::reset()
{
  this->clear();
  rewind();
}

/*
 _____         _     __  __       _       
|_   _|__  ___| |_  |  \/  | __ _(_)_ __  
  | |/ _ \/ __| __| | |\/| |/ _` | | '_ \ 
  | |  __/\__ \ |_  | |  | | (_| | | | | |
  |_|\___||___/\__| |_|  |_|\__,_|_|_| |_|
                                          
*/

#ifndef CNCPP_TEST_PROGRAM_MAIN

#include <rang.hpp>
using namespace fmt;

int main(int argc, const char *argv[]) {
  if (argc != 3) {
    cerr << log_tag(LogType::ERROR) << " Usage: " << argv[0] << " <machine.json> <file.g>" << endl;
    return EXIT_FAILURE;
  }

  string machine_data{argv[1]};

  Machine machine{};
  try {
    machine = Machine(machine_data);
  } catch (const exception &e) {
    cerr << log_tag(LogType::ERROR) << e.what() << endl;
    return EXIT_FAILURE;
  }

  cerr << log_tag(LogType::MESSAGE) << " Machine parameters: " << endl << machine << endl;

  Program program{&machine};
  try {
    program.load(argv[2]);
  } catch (exception &e) {
    cerr << log_tag(LogType::ERROR) << " Failed to load program: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  cerr << log_tag(LogType::MESSAGE) << " Program summary: " << endl
       << program << endl;

  cerr << log_tag(LogType::MESSAGE) << "Sequence of position (to stdout only):" << endl;
  cout << "n,t_tot,t,lambda,s,x,y,z" << endl;
  // Loop over all the blocks here:
  data_t t_tot = 0.0;
  for (auto &block : program) {
    // skip rapid/nomotion blocks because those are not interpolated
    if (block.type() == Block::BlockType::RAPID || block.type() == Block::BlockType::NO_MOTION) continue;
    // loop within a block
    block.walk([&](Block &b, data_t t, data_t l, data_t s) {
      Point pos = b.interpolate(l);
      cout << format("{:},{:},{:},{:},{:},{:},{:},{:}", b.n(), t_tot, t, l, s,
                    pos.x(), pos.y(), pos.z())
          << endl;
      t_tot += machine.tq();
    });
  }

  return EXIT_SUCCESS;
}

#endif // CNCPP_TEST_PROGRAM_MAIN