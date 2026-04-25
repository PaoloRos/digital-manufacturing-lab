/*
 ____                                                   ____ _               
|  _ \ _ __ ___   __ _ _ __ __ _ _ __ ___  _ __ ___    / ___| | __ _ ___ ___ 
| |_) | '__/ _ \ / _` | '__/ _` | '_ ` _ \| '_ ` _ \  | |   | |/ _` / __/ __|
|  __/| | | (_) | (_| | | | (_| | | | | | | | | | | | | |___| | (_| \__ \__ \
|_|   |_|  \___/ \__, |_|  \__,_|_| |_| |_|_| |_| |_|  \____|_|\__,_|___/___/
                 |___/                                                       

Class representing the G-code program, as a list of Block istances.

Author: Paolo Rossi
Date: 2026-04-25
*/

#pragma once

#include "defines.hpp"
#include "block.hpp"
#include "machine.hpp"

#include <list>

namespace cncpp {

class Program : public Object, public std::list<Block>
{
  public:
  using iterator = std::list<Block>::iterator;        // alias for list::iterator type

  // LIFECYCLE =================================================================

  Program(std::string &f, Machine *m);
  Program(Machine *m);
  ~Program();
  std::string desc(bool colored = true) const override;

  // OPERATIONS/OPERATORS ======================================================

  void load(std::string &f, bool append = false);  // load the program from file, parsing each block and filling the list of blocks. If append is true, new blocks are added to the end of the list, otherwise the list is cleared before loading.
  Program &operator<<(std::string const &line);
  iterator load_next();
  void rewind();
  void reset(); // se fa il list.clear, perché non chiamarla direttamente?

  // ACCESSORS =================================================================
  iterator current() const { return _current; }
  bool done() const { return _done; }

  private:
  Machine *_m = nullptr;      // Pointer to a read-only machine
  std::string _filename = "";       // Source file name
  iterator _current = this->end();  // Iterator pointing to the current block being executed
  bool _done = false;               // Flag indicating whether the program execution is completed

};

}  // namespace cncpp