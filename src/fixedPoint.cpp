#include "fixedPoint.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <string>
#include <stdexcept>


Fixed_t::Fixed_t(size_t precision){
  set_precision(precision);
}
Fixed_t::Fixed_t(int32_t number, size_t precision) : number(number){
  set_precision(precision);
}
Fixed_t::Fixed_t(double number, size_t precision){
  set_precision(precision);
  this->number = (int32_t)number;
  if(digits.size() >= 1){
    // (double to uint64_t behaves differently between amd64 and aarch64!)
    // aarch64: negative double values get casted to 0 if casted directly to uint64_t
    // amd64: double to uint64_t cast behaves just like double to int64_t and keeps the sign bit.
    double tmp = (std::ldexp(number - (double)this->number, 63));
    digits[0] = (uint64_t)((int64_t)tmp) << 1; //get around the sign bit in the double to int conversion 
    if(digits[0] != 0 && number < 0.0){
      this->number --;//Handle 2s complement
    }
  }
}

Fixed_t::Fixed_t(std::string hex){
  size_t idx = hex.find_first_of('.') + 1;
  if(hex[1] != '0' || hex[2] != 'x' || idx == 0)
    throw std::invalid_argument("Hex string must start with 0x and contain a '.'!");
  number = std::stoi(hex.substr(3, idx - 2));
  while(idx < hex.size()){
    digits.push_back(std::stoull(hex.substr(idx, 16), 0, 16));
    idx += 16;
  }
  if(hex[0] == '-')
    this->neg();
  else if(hex[0] != '+')
    throw std::invalid_argument("Hex string must start with '+' or '-'!");
}


void Fixed_t::set_precision(size_t precision){
  digits.resize((precision + (DBITS - 1)) / DBITS);
}

std::string Fixed_t::toHex() const{
  std::stringstream res;
  Fixed_t x = *this;
  if(number < 0){
    x = -x;
    res << "-0x";
  }else
    res << "+0x";
  res << std::hex << x.number << '.';
  for(size_t i = 0; i < x.digits.size(); i++){
    res << std::setw(16) << std::setfill('0') << std::hex << x.digits[i];
  }
  return res.str();
}
