/*
Program class implementation.

Author: Paolo Rossi
Date: 2026-04-25
*/

#include "defines.hpp"
#include "program.hpp"
#include <fstream>

using namespace std;

using namespace cncpp;

// LIFECYCLE ===================================================================

Program::Program(string &f, Machine *m) : _filename(f), _m(m)
{
  load(_filename);  // in load si aggiorna nuovamente l'attributo _filename!

  cerr << log_tag(LogType::MESSAGE, cerr)
       << " Program " << _filename << " created" << endl;
}

Program::Program(Machine *m) : _m(m)
{
  cerr << log_tag(LogType::MESSAGE, cerr)
       << " Program created with no source file" << endl;
}

Program::~Program()
{
  cerr << log_tag(LogType::MESSAGE, cerr)
       << " Program " << _filename << " destroyed" << endl;
}

string Program::desc(bool colored) const {
  cerr << log_tag(LogType::WARNING) << " Program::desc() to be implemented!" << endl;
  return "";
}

// OPERATIONS/OPERATORS ========================================================

void Program::load(string &f, bool append)
{
  cerr << log_tag(LogType::MESSAGE, cerr)
       << " Loading program from file " << f << "..." << endl;

  _filename = f;  // aggiornamento ripetitivo considerando il costruttore di default

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
