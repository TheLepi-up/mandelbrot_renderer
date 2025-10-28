
#ifndef FIXTED_T_SETCALC
#define FIXTED_T_SETCALC

#include "calculate.h"
#include "fixedPoint.h"


class Fixed_SetCalc : public SetCalc
{
private:
  Fixed_t xPos;
  Fixed_t yPos;
public:
  Fixed_SetCalc();
  Fixed_SetCalc(std::string initial_pos);
  ~Fixed_SetCalc() override;
  uint64_t calculate(double x, double y, uint64_t depth) override;
  void zoomIn(double x, double y) override;
  void zoomOut(double x, double y) override;
  void reset() override;
  void fromStr(std::string pos) override;
  const std::string toString() override;
};


#endif
