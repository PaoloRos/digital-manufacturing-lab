/*
Machine class implementation.

Author: Paolo Rossi
Date: 2026-04-28
*/

#include "machine.hpp"

using namespace cncpp;

using namespace std;
using json = nlohmann::json;

// LIFECYCLE ===================================================================

Machine::Machine(nlohmann::json &j) {}

Machine::Machine(std::string &filename) {}

std::string Machine::desc(bool colored) const
{
  cerr << log_tag(LogType::WARNING) << "Machine::desc not implemented yet" << endl;
  return "";
}

// OPERATIONS/OPERATORS ========================================================

void Machine::load(nlohmann::json &j) 
{
  cerr << log_tag(LogType::WARNING) << "Machine::load not implemented yet" << endl;
}

data_t Machine::quantize(data_t t, data_t &dq) const
{
  data_t q;                                   // next quantized time
  q = static_cast<size_t>( t/_tq + 1) * _tq;  // q > t
  dq = q - t;                                 // quantization error
  return q;
}