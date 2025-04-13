#ifndef TPGTCFloats_hh
#define TPGTCFloats_hh

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "TPGTCBits.hh"
#include "TMath.h"

// Code from Paul, dated 06/11/2024 (updated 12/04/2025 by Indra)

class TPGTCFloats : public TPGTCBits {
public:
  TPGTCFloats() {
    TPGTCBits::setZero();
  }

  TPGTCFloats(const TPGTCBits &w) : TPGTCBits(w) {
  }

  double getEnergyGeV() const {
    return TPGTCBits::getEnergy()/_eScale;
  }

  double getPhiF() const {
    double phi = TPGTCBits::getPhi()/_phiScale ;
    if(phi>TMath::Pi()) phi -= 2*TMath::Pi();
    return phi;
  }

  double getXOverZF() const {
    return TPGTCBits::getROverZ()*cos(getPhiF())/_rozScale;
  }

  double getYOverZF() const {
    return TPGTCBits::getROverZ()*sin(getPhiF())/_rozScale;
  }

  double getZCm() const {
    uint16_t l(getLayer());
    return l>0 && l<48?_zLayer[l-1]:0.0;
  }

  void setEnergyGeV(double e) {
    setEnergy(uint32_t(_eScale*e+0.5));
  }
  
  // void setXOverZF(double x) {
  //   setXOverZ(int16_t(x>=0.0?_xyScale*x+0.5:_xyScale*x-0.5));
  // }
  
  // void setYOverZF(double y) {
  //   setYOverZ(int16_t(y>=0.0?_xyScale*y+0.5:_xyScale*y-0.5));
  // }
  
  // void setXYOverZF(double x, double y, unsigned s) {
  //   static double s120(0.5*sqrt(3.0)),c120(-0.5);
  
  //   if(     s==0) {setXOverZF( y            );setYOverZF(-x            );}
  //   else if(s==1) {setXOverZF( y*c120-x*s120);setYOverZF(-x*c120-y*s120);}
  //   else if(s==2) {setXOverZF( y*c120+x*s120);setYOverZF(-x*c120+y*s120);}
  //   else if(s==3) {setXOverZF( y            );setYOverZF( x            );}
  //   else if(s==4) {setXOverZF( y*c120+x*s120);setYOverZF( x*c120-y*s120);}
  //   else if(s==5) {setXOverZF( y*c120-x*s120);setYOverZF( x*c120+y*s120);}
  
  //   else assert(false);
  // }
  
  void setROverZF(double roz) {
    setROverZ(uint16_t(roz*_rozScale+0.5));
  }
  
  void setPhiF(double phi) {
    if(phi<0.0) phi += 2*TMath::Pi(); 
    setPhi(uint16_t(phi*_phiScale+0.5));
  }
  
  void setROverZPhiF(double x/*xoz*/, double y/*yoz*/, unsigned s) {
    static double s120(0.5*sqrt(3.0)),c120(-0.5);
    static double xl,yl;//local refframe
    
    switch(s){
    case 0:
      xl = y ; yl = -x;      
      break;
    case 1:
      xl = y*c120-x*s120 ; yl = -x*c120-y*s120;
      break;
    case 2:
      xl = y*c120+x*s120 ; yl = -x*c120+y*s120;
      break;
    case 3:
      xl = y ; yl = x;
      break;
    case 4:
      xl = y*c120+x*s120 ; yl = x*c120-y*s120;
      break;
    case 5:
      xl = y*c120-x*s120 ; yl = x*c120+y*s120;
      break;
    default:
      assert(false);
    }
    //if(TMath::AreEqualAbs(xl,0.0,1.e-5)) std::cout << "TPGTCFloats::setROverZPhiF : (x,y) : (" << x << ", " << y <<"), s: " << s << ", (xl,yl) : (" << xl <<", " << yl << ") "<< std::endl;
    //assert(!TMath::AreEqualAbs(xl,0.0,1.e-5));
    
    setROverZF(sqrt(xl*xl+yl*yl));
    setPhiF(atan2(yl,xl));
  }

  void print() {
    std::cout << "TPGTCFloats(" << this << ")::print()"
	      << std::endl << " ";
    TPGTCBits::print();
    std::cout << " Energy " << getEnergyGeV() << " GeV" << std::endl
	      << " x/z, y/z, z = " << getXOverZF()
	      << ", " << getYOverZF()
	      << ", " << getZCm() << std::endl;
  }

private:
  static double _zLayer[47];
  static double _eScale;
  //static double _xyScale;
  static double _rozScale;
  static double _phiScale;
};

double TPGTCFloats::_zLayer[]={
  322.155,     302, 325.212,     304, 328.269,     306, 331.326,     308, 334.383,     310,
  337.440,     312, 340.497,     314, 343.554,     316, 346.611,     318, 349.993,     320,
  353.375,     322, 356.757,     324, 360.139,     326, 367.976,     374, 380.586, 386.891,
  393.196, 399.501, 405.806, 412.111, 418.416, 424.721, 431.026, 439.251, 447.476, 455.701,
  463.926, 472.151, 480.376, 488.406, 496.826, 505.051, 513.276
};

double TPGTCFloats::_eScale=(1<<18)/512.; //(max value)/(max of float range)
//double TPGTCFloats::_xyScale=2500.0;
double TPGTCFloats::_rozScale=(1<<12)/0.575;
double TPGTCFloats::_phiScale=(1<<12)/(2*TMath::Pi());

#endif
