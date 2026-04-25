/*
 __  __            _     _               ____ _               
|  \/  | __ _  ___| |__ (_)_ __   ___   / ___| | __ _ ___ ___ 
| |\/| |/ _` |/ __| '_ \| | '_ \ / _ \ | |   | |/ _` / __/ __|
| |  | | (_| | (__| | | | | | | |  __/ | |___| | (_| \__ \__ \
|_|  |_|\__,_|\___|_| |_|_|_| |_|\___|  \____|_|\__,_|___/___/
                                                             

Machine definition and interface.

Author: Paolo Rossi
Date: 2026-04-25
*/

#pragma once
#include "defines.hpp"
#include "point.hpp"

namespace cncpp {

/**
 * @brief Minimal machine interface used by Block during parsing/interpolation.
 */
class Machine { 
  public:
  /**
   * @brief Get the machine reference point.
   * @return Machine origin point.
   */
  Point zero() const { return Point(0.0, 0.0, 0.0); }
  /**
   * @brief Get machine control period.
   * @return Time quantum in seconds.
   */
  data_t tq() const { return _tq; }
  /**
   * @brief Get machine maximum acceleration.
   * @return Maximum acceleration value.
   */
  data_t A() const { return 1000.0; }
  /**
   * @brief Get the maximum allowed positioning error.
   * @return Positioning tolerance in millimeters.
   */
  data_t error() const { return 1e-3; }
  /**
   * @brief Quantize a time value to the next control tick.
   * @param t Input time value.
   * @param dq Output quantization error, computed as @f$q - t@f$.
   * @return Next quantized time @f$q@f$, with @f$q > t@f$.
   */
  data_t quantize(data_t t, data_t &dq) const;
  
  private:
  data_t _tq = 0.001;  // control period in seconds (quantization step for time)
};

inline data_t Machine::quantize(data_t t, data_t &dq) const {
  data_t q;                                   // next quantized time
  q = static_cast<size_t>( t/_tq + 1) * _tq;  // q > t
  dq = q - t;                                 // quantization error
  return q;
}
};  // namespace cncpp