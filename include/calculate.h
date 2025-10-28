
#ifndef SETCALC
#define SETCALC

#include <string>
#include <inttypes.h>
#include <stdexcept>

class SetCalc
{
protected:
  int64_t scale = -1;
public:
  virtual ~SetCalc(){};
  virtual uint64_t calculate(double x, double y, uint64_t depth) = 0;
  virtual void zoomIn(double x, double y) = 0;
  virtual void zoomOut(double x, double y) = 0;
  virtual void reset() = 0;
  virtual void fromStr(std::string pos) = 0;
  virtual const std::string toString() = 0;
};

#endif