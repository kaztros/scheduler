//
//  Error.cpp
//  
//
//  Created by kaz on 10/26/20.
//

#include "Error.hpp"

Error NO_ERROR = std::make_shared<NoError>();

std::string prepend_comma(std::string v) {
  return ", " + v;
}


// Need two arguments to prevent clash with overloaded function.
template <typename T_0, typename T_1, typename ... T_REST>
std::string to_string (T_0 first, T_1 second, T_REST ... rest) {
  return to_string (first) + prepend_comma(to_string(second)) + (prepend_comma(to_string(rest)) + ...);
}


template <typename ... T>
std::string to_string (std::tuple<T...> const & x) {
  return std::apply (to_string, x);
}

std::ostream& NoError::PrintVia (std::ostream& printer) const {
  return printer << "No error!";
}

