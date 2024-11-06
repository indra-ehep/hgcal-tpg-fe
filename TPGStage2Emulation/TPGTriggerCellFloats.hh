#ifndef TPGTriggerCellFloats_hh
#define TPGTriggerCellFloats_hh

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "TPGTriggerCellWord.hh"

// Code from Paul, dated 06/11/2024

class TPGTriggerCellFloats : public TPGTriggerCellWord {
public:
  TPGTriggerCellFloats() {
    TPGTriggerCellWord::setZero();
  }

  TPGTriggerCellFloats(const TPGTriggerCellWord &w) : TPGTriggerCellWord(w) {
  }

  float getEnergyGeV() const {
    return TPGTriggerCellWord::getEnergy()/_eScale;
  }

  float getXOverZF() const {
    return TPGTriggerCellWord::getXOverZ()/_xyScale;
  }

  float getYOverZF() const {
    return TPGTriggerCellWord::getYOverZ()/_xyScale;
  }

  float getZCm() const {
    uint16_t l(getLayer());
    return l>0 && l<48?_zLayer[l-1]:0.0;
  }

  void setEnergyGeV(float e) {
    setEnergy(uint32_t(_eScale*e+0.5));
  }
  
  void setXOverZF(float x) {
    setXOverZ(int16_t(x>=0.0?_xyScale*x+0.5:_xyScale*x-0.5));
  }
  
  void setYOverZF(float y) {
    setYOverZ(int16_t(y>=0.0?_xyScale*y+0.5:_xyScale*y-0.5));
  }
  
  void setXYOverZF(float x, float y, unsigned s) {
    static double sn(0.5*sqrt(3.0)),cs(-0.5);

    if(     s==0) {setXOverZF( x        );setYOverZF( y        );}
    else if(s==1) {setXOverZF( x*cs+y*sn);setYOverZF( y*cs-x*sn);}
    else if(s==2) {setXOverZF( x*cs-y*sn);setYOverZF( y*cs+x*sn);}
    else if(s==3) {setXOverZF(-x        );setYOverZF( y        );}
    else if(s==4) {setXOverZF(-x*cs+y*sn);setYOverZF( y*cs+x*sn);}
    else if(s==5) {setXOverZF(-x*cs-y*sn);setYOverZF( y*cs-x*sn);}

    else assert(false);
  }
  
  void print() {
    std::cout << "TPGTriggerCellFloats(" << this << ")::print()"
	      << std::endl << " ";
    TPGTriggerCellWord::print();
    std::cout << " Energy " << getEnergyGeV() << " GeV" << std::endl
	      << " x/z, y/z, z = " << getXOverZF()
	      << ", " << getYOverZF()
	      << ", " << getZCm() << std::endl;
  }

private:
  static float _zLayer[47];
  static float _eScale;
  static float _xyScale;
};

float TPGTriggerCellFloats::_zLayer[]={
  322.155,     302, 325.212,     304, 328.269,     306, 331.326,     308, 334.383,     310,
  337.440,     312, 340.497,     314, 343.554,     316, 346.611,     318, 349.993,     320,
  353.375,     322, 356.757,     324, 360.139,     326, 367.976,     374, 380.586, 386.891,
  393.196, 399.501, 405.806, 412.111, 418.416, 424.721, 431.026, 439.251, 447.476, 455.701,
  463.926, 472.151, 480.376, 488.406, 496.826, 505.051, 513.276
};

float TPGTriggerCellFloats::_eScale=256.0;
float TPGTriggerCellFloats::_xyScale=2500.0;

#endif
