#include "fixed_t_calc.h"
#include <stdexcept>
#include <sstream>
#include <iostream>

#define SCALE_PREC (this->scale + 32)

Fixed_SetCalc::Fixed_SetCalc(std::string initial_pos){
  fromStr(initial_pos);
}
Fixed_SetCalc::Fixed_SetCalc(){
  reset();
}

Fixed_SetCalc::~Fixed_SetCalc()
{
}

uint64_t Fixed_SetCalc::calculate(double x, double y, uint64_t depth){
  Fixed_t c_re(x, SCALE_PREC);
  Fixed_t c_im(y, SCALE_PREC);
  c_re >>= scale;
  c_im >>= scale;
  c_re += xPos;
  c_im += yPos;

  uint64_t i = 0;
  Fixed_t z_re(SCALE_PREC), z_im(SCALE_PREC), old_z_re(SCALE_PREC), z_im_sq(SCALE_PREC);
  Fixed_t ptwo(2, 0);
  Fixed_t ntwo(-2, 0);
  while (z_re > ntwo && z_re < ptwo && z_im > ntwo && z_im < ptwo && i < depth)
  {
      //z_re = z_re * z_re - z_im * z_im + c_re;
      //z_im = ((old_z_re * z_im) << 1) + c_im;
      //old_z_re = z_re;
      //i++;
      
      z_re *= z_re;
      z_im_sq.copyFrom(z_im);
      z_im_sq *= z_im;
      z_re -= z_im_sq;
      z_re += c_re;
      
      z_im *= old_z_re;
      z_im <<= 1;
      z_im += c_im;

      old_z_re.copyFrom(z_re);
      i++;
  }
  // long double z0 = 0.0;
  // long double z1 = 0.0;
  // long double z2 = 0.0;
  // long double z3 = 0.0;
  // long double c0 = x;
  // long double c1 = 0.0;
  // long double c2 = y;
  // long double c3 = 0.0;
  // while (abs(z0) < 2 && abs(z1) < 2 && abs(z2) < 2 && i < depth)
  // {
  //     long double t0 = z0*z0 - 2*z1*z3 - z2*z2 + c0;
  //     long double t1 = 2*z0*z1 - 2*z2*z3 + c1;
  //     long double t2 = 2*z0*z2 + z1*z1 - z3*z3 + c2;
  //     long double t3 = 2*z0*z3 + 2*z1*z2 + c3;
  //     z0 = t0;
  //     z1 = t1;
  //     z2 = t2;
  //     z3 = t3;
  //     i++;
  // }
  return i;
}

void Fixed_SetCalc::zoomIn(double x, double y){
  xPos += Fixed_t(x, SCALE_PREC) >> scale;
  yPos += Fixed_t(y, SCALE_PREC) >> scale;
  scale ++;
}
void Fixed_SetCalc::zoomOut(double x, double y){
  xPos += Fixed_t(x, SCALE_PREC) >> scale;
  yPos += Fixed_t(y, SCALE_PREC) >> scale;
  scale --;
  xPos.set_precision(SCALE_PREC);
  yPos.set_precision(SCALE_PREC);
}
void Fixed_SetCalc::reset(){
  xPos = Fixed_t(0, 31);
  yPos = Fixed_t(0, 31);
  scale = -1;
}

void Fixed_SetCalc::fromStr(std::string pos){
  size_t yIdx = pos.find_first_of('_') + 1;
  size_t scaleIdx = pos.find_first_of('_', yIdx) + 1;
  if(yIdx == 0 || scaleIdx == 0)
    throw std::invalid_argument("Position must contain 2 '_'");
  if(pos[0] != 'x' || pos[1] != ':'){
    throw std::invalid_argument("x-Position must start with 'x:'");
  }
  if(pos[yIdx] != 'y' || pos[yIdx + 1] != ':'){
    throw std::invalid_argument("y-Position must start with 'y:'");
  }
  if(pos.find("scale:", yIdx) != scaleIdx){
    throw std::invalid_argument("scale must start with 'scale:'");
  }
  xPos = Fixed_t(pos.substr(2, yIdx - 3));
  yPos = Fixed_t(pos.substr(yIdx + 2, scaleIdx - yIdx - 3));
  scale = std::stoll(pos.substr(scaleIdx + 6));
}

const std::string Fixed_SetCalc::toString() {
  std::stringstream name;
  name << "x:" << xPos.toHex() << "_y:" << yPos.toHex() << "_scale:" << scale;
  
  return name.str();
}