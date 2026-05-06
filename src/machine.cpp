/*
Machine class implementation.

Author: Paolo Rossi
Date: 2026-04-28
*/

#include "machine.hpp"
#include <fstream>
#include <sstream>

using namespace cncpp;
using namespace std;
using json = nlohmann::json;

// LIFECYCLE ===================================================================

Machine::Machine(json &j) { load(j); }

Machine::Machine(std::string &filename) { load(filename); }

std::string Machine::desc(bool colored) const
{
  stringstream ss;
  ss << "A = " << _A << ", "
     << "tp = " << _tq << ", "
     << "max_error = " << _max_error << ", "
     << "fmax = " << _fmax
     << "zero = " << _zero.desc(colored) << ", "
     << "offset = " << _offset.desc(colored) << endl;
  return ss.str();
}

// OPERATIONS/OPERATORS ========================================================

void Machine::load(json &j) 
{
  _data = j;
  
  if (_data.contains("tq")) {
    if (!_data["tq"].is_number())
      throw invalid_argument("Machine parameter 'tq' must be a number");
    _tq = _data["tq"];
  }
  if (_data.contains("A")) {
    if (!_data["A"].is_number())
      throw invalid_argument("Machine parameter 'A' must be a number");
    _A = _data["A"];
  }
  if (_data.contains("fmax")) {
    if (!_data["fmax"].is_number())
      throw invalid_argument("Machine parameter 'fmax' must be a number");
    _fmax = _data["fmax"];
  }
  if (_data.contains("max_error")) {
    if (!_data["max_error"].is_number())
      throw invalid_argument("Machine parameter '_max_error' must be a number");
    _max_error = _data["max_error"];
  }
  if (_data.contains("zero")) {
    if (!_data["zero"].is_array() || _data["zero"].size()!=3)
      throw invalid_argument("Machine parameter 'zero' must be an array of 3 numbers");
    for (auto &v : _data["zero"]) {
      if (!v.is_number()) 
        throw invalid_argument("Machine parameter 'zero' must be an array of 3 numbers");
    }
    _zero.x(_data["zero"][0]);
    _zero.y(_data["zero"][1]);
    _zero.z(_data["zero"][2]);
  }
  if (_data.contains("offset")) {
    if (!_data["offset"].is_array() || _data["offset"].size()!=3)
      throw invalid_argument("Machine parameter 'offset' must be an array of 3 numbers");
    for (auto &v : _data["offset"]) {
      if (!v.is_number()) 
        throw invalid_argument("Machine parameter 'offset' must be an array of 3 numbers");
    }
    _offset.x(_data["offset"][0]);
    _offset.y(_data["offset"][1]);
    _offset.z(_data["offset"][2]);
  }
}

void Machine::load(string &filename)
{
  ifstream f(filename);
  json data = json::parse(f);
  load(data);
}

data_t Machine::quantize(data_t t, data_t &dq) const
{
  data_t q;                                   // next quantized time
  q = static_cast<size_t>( t/_tq + 1) * _tq;  // q > t
  dq = q - t;                                 // quantization error
  return q;
}

/*
 _____         _     __  __       _       
|_   _|__  ___| |_  |  \/  | __ _(_)_ __  
  | |/ _ \/ __| __| | |\/| |/ _` | | '_ \ 
  | |  __/\__ \ |_  | |  | | (_| | | | | |
  |_|\___||___/\__| |_|  |_|\__,_|_|_| |_|
                                          
*/

#ifdef CNCPP_MACHINE_TEST_MAIN
#include <iostream>

int main(int argc, const char **argv) {
  if (argc != 2) {
    cerr << log_tag(LogType::ERROR) << " Usage: " << argv[0] << " <machine.json>" << endl;
    return EXIT_FAILURE;
  }
  string filename{argv[1]};
  Machine m{};
  try {
    m = Machine(filename);
  } catch (const exception &e) {
    cerr << log_tag(LogType::ERROR) << e.what() << endl;
    return EXIT_FAILURE;
  }
  
  cout << log_tag(LogType::MESSAGE) << " Parsed JSON data structure: " << endl
       << m.data().dump(2) << endl;

  cout << log_tag(LogType::MESSAGE)
       << "Machine parameters: " << endl << m << endl;

  return 0;
}



#endif // CNCPP_MACHINE_TEST_MAIN