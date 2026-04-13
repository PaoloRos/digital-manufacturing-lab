#include "damper.hpp"

ConfigErrors Damper::set_config(const std::map<std::string, double>& config)
{
  ConfigErrors errors;

  // Setting damping coefficient c
  auto c_it = config.find("c");
  if (c_it == config.end()) { 
    errors.push_back( {ConfigErrorCode::MISSING_KEY, "c", std::nullopt} );
  } else if (c_it->second <= 0.0) { 
    errors.push_back( {ConfigErrorCode::INVALID_VALUE, "c", c_it->second} );
  } else{ 
    _c = c_it->second; 
  }

  // Setting mass m
  auto m_it = config.find("m");
  if (m_it == config.end()) { 
    errors.push_back( {ConfigErrorCode::MISSING_KEY, "m", std::nullopt} );
  } else if (m_it->second <= 0.0) { 
    errors.push_back( {ConfigErrorCode::INVALID_VALUE, "m", m_it->second} );
  } else{ 
    _m = m_it->second; 
  }

  return errors;
}