/*
 ____                                                   ____ _
|  _ \ _ __ ___   __ _ _ __ __ _ _ __ ___  _ __ ___    / ___| | __ _ ___ ___
| |_) | '__/ _ \ / _` | '__/ _` | '_ ` _ \| '_ ` _ \  | |   | |/ _` / __/ __|
|  __/| | | (_) | (_| | | | (_| | | | | | | | | | | | | |___| | (_| \__ \__ \
|_|   |_|  \___/ \__, |_|  \__,_|_| |_| |_|_| |_| |_|  \____|_|\__,_|___/___/
                 |___/

G-code program container implemented as a list of Block instances.

Author: Paolo Rossi
Date: 2026-04-25
*/

#pragma once

#include "defines.hpp"
#include "block.hpp"
#include "machine.hpp"

#include <list>

namespace cncpp {
/** @brief Iterator alias for the underlying list of blocks. */
using block_iterator = std::list<Block>::iterator;

class Program : public Object, public std::list<Block>
{
  public:

  // LIFECYCLE =================================================================

  /**
   * @brief Build and load a program from a file.
   * @param f Path to the G-code file.
   * @param m Machine context used for parsing blocks.
   */
  Program(std::string const &f, Machine *m);
  /**
   * @brief Build an empty program bound to a machine.
   * @param m Machine context used for parsing blocks.
   */
  Program(Machine *m);
  /** @brief Destroy the program container. */
  ~Program();
  /**
   * @brief Build a string description of the program.
   * @param colored Enable ANSI colored output when true.
   * @return Human-readable program summary.
   */
  std::string desc(bool colored = true) const override;

  // OPERATIONS/OPERATORS ======================================================

  /**
   * @brief Load a G-code program from file.
   * @param f Path to the G-code file.
   * @param append When true, append to the current list; otherwise reset first.
   */
  void load(std::string const &f, bool append = false);
  /**
   * @brief Append a raw G-code line as a new block and parse it.
   * @param line Raw G-code line.
   * @return Reference to this program.
   */
  Program &operator<<(std::string const &line);
  /**
   * @brief Advance the current iterator and return it.
   * @return Iterator to the current block after advancing.
   */
  block_iterator load_next();
  /** @brief Rewind the current iterator to the first block. */
  void rewind();
  /** @brief Clear the program and reset iteration state. */
  void reset();

  // ACCESSORS =================================================================
  /** @brief Get iterator to the current block. */
  block_iterator current() const { return _current; }
  /** @brief Check whether the iterator reached the end. */
  bool done() const { return _done; }

  private:
  // Machine context used while parsing blocks.
  Machine *_m = nullptr;
  // Source file name used when loading from disk.
  std::string _filename = "";
  // Iterator pointing to the current block during iteration.
  block_iterator _current = this->end();
  // Flag indicating whether iteration has completed.
  bool _done = false;

};

}  // namespace cncpp