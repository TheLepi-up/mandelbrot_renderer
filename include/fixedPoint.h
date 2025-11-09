#ifndef FIXEDPOINT
#define FIXEDPOINT
#include <vector>
#include <inttypes.h>
#include <string>
#include <memory.h>

typedef unsigned int uint;
#define DBITS (sizeof(uint64_t) * 8)
#define PREC(bits) ((bits + (DBITS - 1)) / DBITS)

class DigitList{
private:
  uint64_t* _data = nullptr;
  size_t _size = 0;
public:
  inline uint64_t &operator[](const size_t idx) const { return _data[idx]; }
  inline const size_t size() const { return _size; }
  inline void resize(const size_t newSize){ 
    if(newSize == _size)
      return;
    if(newSize == 0){
      _size = newSize;
      free(_data);
      _data = nullptr;
      return;
    }
    if(_data)
      _data = (uint64_t*)realloc(_data, newSize * sizeof(uint64_t));
    else
      _data = (uint64_t*)malloc(newSize * sizeof(uint64_t));
    for (size_t i = _size; i < newSize; i++)
      _data[i] = 0;//initialize new memory
    _size = newSize;
  }
  inline DigitList(){}
  inline DigitList(const size_t size){ resize(size); }
  inline DigitList(const DigitList& other){ 
    resize(other._size);
    memcpy(_data, other._data, _size * sizeof(uint64_t));
  }
  inline ~DigitList(){
    free(_data);
    _data = nullptr;
    _size = 0;
  }
};

class Fixed_t{
private:
  int32_t number = 0;
  DigitList digits;

  public:
  inline Fixed_t(){};
  inline Fixed_t(const Fixed_t& other) : number(other.number), digits(other.digits){};
  inline Fixed_t(size_t precision) : digits(PREC(precision)) {}
  inline Fixed_t(int32_t number, size_t precision) : number(number), digits(PREC(precision)){}
  Fixed_t(double number, size_t precision);
  Fixed_t(std::string hex);
  inline void copyFrom(Fixed_t const& other);
  inline void operator=(int32_t number);
  void operator=(const double number);
  void operator=(const std::string hex);
  //Do not use this by accedent as it is inefficient in most cases
  void operator=(const Fixed_t& other) = delete; 
  inline bool operator<(Fixed_t const& other) const;
  inline bool operator>(Fixed_t const& other) const;
  inline bool operator==(Fixed_t const& other) const;
  inline void operator+=(Fixed_t const& other);
  inline void operator-=(Fixed_t const& other);
  inline void neg();
  inline void operator*=(Fixed_t & other);
  inline void operator/=(Fixed_t const& other);
  inline void operator>>=(int other);
  inline void operator<<=(int other);
  inline int8_t compare(Fixed_t const& other) const;
  inline Fixed_t operator+(Fixed_t const& other) const;
  inline Fixed_t operator-(Fixed_t const& other) const;
  inline Fixed_t operator-() const;
  inline Fixed_t operator*(Fixed_t &other) const;
  inline Fixed_t operator/(Fixed_t const& other) const;
  inline Fixed_t operator<<(int other) const;
  inline Fixed_t operator>>(int other) const;
  inline void set_precision(size_t precision){ digits.resize(PREC(precision)); }
  std::string toHex() const;
};

inline void Fixed_t::copyFrom(Fixed_t const& other){
  digits.resize(other.digits.size());
  number = other.number;
  for (size_t i = 0; i < other.digits.size(); i++)
  {
    digits[i] = other.digits[i];
  }
}

inline void Fixed_t::operator=(int32_t number){
  this->number = number;
  for (size_t i = 0; i < digits.size(); i++)
  {
    digits[i] = 0;
  }
}

inline int8_t Fixed_t::compare(Fixed_t const& other) const{
  if(number < other.number)
    return -1;
  else if(number > other.number)
    return 1;
  uint minPrec = std::min(digits.size(), other.digits.size());
  for(uint i = 0; i < minPrec; i++){
    if(digits[i] < other.digits[i])
      return -1;
    else if(digits[i] > other.digits[i])
      return 1;
  }
  uint maxPrec = std::max(digits.size(), other.digits.size());
  bool endA = digits.size() != maxPrec;
  for(uint i = minPrec; i < maxPrec; i++){
    uint64_t a = endA ? 0 : digits[i];
    uint64_t b = endA ? other.digits[i] : 0;
    if(a < b)
      return -1;
    else if(b < a)
      return 1;
  }
  return 0;
}

inline bool Fixed_t::operator<(Fixed_t const& other) const{
  return compare(other) < 0;
}
inline bool Fixed_t::operator>(Fixed_t const& other) const{
  return compare(other) > 0;
}
inline bool Fixed_t::operator==(Fixed_t const& other) const{
  return compare(other) == 0;
}
inline void Fixed_t::operator+=(Fixed_t const& other) {
  size_t prec = std::max(digits.size(), other.digits.size());
  digits.resize(prec);
  __uint128_t tmp = 0;
  for(int i = other.digits.size() - 1; i >= 0; i--){
    tmp += digits[i];
    tmp += other.digits[i];
    digits[i] = (uint64_t)tmp;
    tmp >>= DBITS;
  }
  number += (int32_t)tmp + other.number;
}

inline void Fixed_t::operator-=(Fixed_t const& other) {
  size_t prec = std::max(digits.size(), other.digits.size());
  digits.resize(prec);
  __uint128_t tmp = 1;
  for(int i = other.digits.size() - 1; i >= 0; i--){
    tmp += digits[i];
    tmp += ~other.digits[i];
    digits[i] = (uint64_t)tmp;
    tmp >>= DBITS;
  }
  number += (int32_t)tmp + ~other.number;
}

inline void Fixed_t::neg(){
  uint carry = 1;
  for (int i = digits.size() - 1; i != -1; i--)
  {
    digits[i] = ~digits[i] + carry;
    if(digits[i] != 0){
      carry = 0;
    }
  }
  number = ~number + carry;
}

#define HI(n) (n >> 32)
#define LO(n) (uint64_t)(uint32_t)n

inline void Fixed_t::operator*=(Fixed_t &other) {
  //multiply with 32bit digits to make the carry fit within the 128bit register
  bool negative = (number < 0) != (other.number < 0);
  if(number < 0) this->neg();
  bool otherneg = false;
  if(other.number < 0){
    other.neg();
    otherneg = true;
  }
  size_t rows = std::min(digits.size(), other.digits.size());
  size_t cols = digits.size();
  __uint128_t tmp = 0;
  //calculate carry
  for (size_t j = 0; j < rows; j++)
  {
    tmp += LO(digits[cols - 1 - j]) * HI(other.digits[j]);
    tmp += HI(digits[cols - 1 - j]) * LO(other.digits[j]);
  }
  tmp >>= 32;
  for (int i = cols - 1; i >= 0; i--)
  {
    //first half
    tmp += LO(digits[i]) * LO(other.number);
    for (int j = 0; j < i; j++)
    {
      tmp += HI(digits[i - j]) * HI(other.digits[j]);
      tmp += LO(digits[i - 1 - j]) * LO(other.digits[j]);
    }
    tmp += HI(digits[0]) * HI(other.digits[i]);
    tmp += LO(number) * LO(other.digits[i]);
    uint32_t res = tmp;
    tmp >>= 32;
    // 2nd half
    tmp += HI(digits[i]) * LO(other.number);
    for (int j = 0; j < i; j++)
    {
      tmp += LO(digits[i - 1 - j]) * HI(other.digits[j]);
      tmp += HI(digits[i - 1 - j]) * LO(other.digits[j]);
    }
    tmp += LO(number) * HI(other.digits[i]);
    digits[i] = (uint64_t)res | (LO(tmp) << 32);//combine the halves
    tmp >>= 32;
  }
  number = number * other.number + tmp;
  if(negative) this->neg();
  if(otherneg) other.neg();
}

inline void Fixed_t::operator>>=(int other){
  if(other < 0){
    *this <<= -other;
    return;
  }
  if(digits.size() == 0){
    number >>= other;
    return;
  }
  size_t offset = other / DBITS;
  size_t frac = other & (DBITS - 1);
  size_t prec = digits.size();
  uint64_t filler = number < 0 ? -1 : 0;//handle 2s complement
  if(frac != 0){//because digits << 64 is a nop.
    for (size_t i = prec - 1; i > offset; i--){
      digits[i] = (digits[i - offset] >> frac) | (digits[i - offset - 1] << (64 - frac));
    }
    if(offset < prec)
      digits[offset] = (digits[0] >> frac) | ((int64_t)number << (64 - frac));
    if(offset >= 1){
      if(offset <= prec)
        digits[offset - 1] = (number >> frac) | (filler << (64 - frac));
      for (size_t i = 0; i < std::min(offset - 1, prec); i++)
      {
        digits[i] = filler;
      }
      number = filler;
    }else{
      number >>= frac;
    }
  }else{
    for (int i = prec - 1; i >= (int)offset; i--){
      digits[i] = digits[i - offset];
    }
    if(offset >= 1){
      if(offset <= prec)
        digits[offset - 1] = number;
      for (size_t i = 0; i < std::min(offset - 1, prec); i++)
      {
        digits[i] = filler;
      }
      number = filler;
    }
  }
}
inline void Fixed_t::operator<<=(int other){
  if(other < 0){
    *this >>= -other;
    return;
  }
  if(digits.size() == 0){
    number <<= other;
    return;
  }
  size_t offset = other / DBITS;
  size_t frac = other & (DBITS - 1);
  size_t prec = digits.size();
  if(frac != 0){
    if(offset >= 1){
      if(offset < prec)
        number = (digits[offset - 1] << frac) | (digits[offset] >> (DBITS - frac));
      else if(offset == prec)
        number = digits[offset - 1] << frac;  
    }else{
      number = (number << frac) | (digits[0] >> (DBITS - frac));
    }
    for (size_t i = 0; i < prec - offset - 1; i++){
      digits[i] = (digits[i + offset] << frac) | (digits[i + offset + 1] >> (DBITS - frac));
    }
    digits[prec - offset - 1] = digits[prec - 1] << frac;
    for (size_t i = prec - offset; i < prec; i++)
    {
      digits[i] = 0;
    }
  }else{
    if(offset >= 1){
      if(offset <= prec)
        number = digits[offset - 1];
      else{
        number = 0;
      }
    }
    for (size_t i = 0; i <= prec - offset - 1; i++){
      digits[i] = digits[i + offset];
    }
    for (size_t i = prec - offset; i < prec; i++)
    {
      digits[i] = 0;
    }
  }

}

inline void Fixed_t::operator/=(Fixed_t const& other) {

}
inline Fixed_t Fixed_t::operator+(Fixed_t const& other) const{
  Fixed_t res(*this);
  res += other;
  return res;
}

inline Fixed_t Fixed_t::operator-(Fixed_t const& other) const{
  Fixed_t res(*this);
  res -= other;
  return res;
}

inline Fixed_t Fixed_t::operator-() const{
  Fixed_t copy(*this);
  copy.neg();
  return copy;
}

inline Fixed_t Fixed_t::operator*(Fixed_t &other) const{
  Fixed_t res(*this);
  res *= other;
  return res;
}
inline Fixed_t Fixed_t::operator/(Fixed_t const& other) const{
  Fixed_t res(*this);
  res /= other;
  return res;
}
inline Fixed_t Fixed_t::operator>>(int other) const{
  Fixed_t res(*this);
  res >>= other;
  return res;
}
inline Fixed_t Fixed_t::operator<<(int other) const{
  Fixed_t res(*this);
  res <<= other;
  return res;
}
#endif