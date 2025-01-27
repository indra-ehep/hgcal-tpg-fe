#ifndef Stage2_h
#define Stage2_h

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <random>
#include <bitset>
#include <algorithm>

#include "TPGBEDataformat.hh"
#include "TPGTriggerCellFloats.hh"
#include "TPGClusterFloats.hh"

#include "CMSSWCode/DataFormats/L1THGCal/interface/HGCalCluster_HW.h"

// Code from Paul, dated 10/11/2024

namespace TPGBEDataformat
{
  typedef unsigned Stage2ToL1tDataArray;
}

namespace TPGStage2Emulation
{

  class TcAccumulator
  {
  public:
    TcAccumulator()
    {
      zero();
    }

    void accumulate(const TPGTriggerCellFloats &t)
    {
      accumulate(t.getEnergy(), t.getXOverZF(), t.getYOverZF(), t.getZCm(), t.getLayer());
    }

    unsigned int getTriggerLayer(const unsigned layer) const {
      bool cee(layer<=26);
      unsigned triggerLayer = 0;
      if (cee) triggerLayer = layer/2+1;
      else triggerLayer = layer-13+1;

      return triggerLayer;
    }

    void accumulate(uint32_t e, double x, double y, double z, int16_t l) {
      bool cee(l<=26);

      unsigned triggerLayer = getTriggerLayer(l);
      const unsigned nTriggerLayers = 34;  // Should get from config/elsewhere in CMSSW
      layerBits_ |= (((unsigned long int)1) << (nTriggerLayers - triggerLayer));
    
      numE++;
      sumE += e;
      ssqE += e * e;

      if (cee)
      {
        numCee++;
        sumCee += e;
        ssqCee += e * e;

        if ( l>6 ) sumCeeCore += e;
      }
      else if (l<=30) sumCehEarly += e;


      double w(e);

      sumW_ += w;

      numX += w;
      sumX += w * x;
      ssqX += w * w * x * x;

      numY += w;
      sumY += w * y;
      ssqY += w * w * y * y;

      unsigned int hw_z = (z-302) / 0.05;
      sumZ_ += w * hw_z;
      ssqZ_ += w * hw_z * hw_z;
    }

    void setSeed(bool b)
    {
      _seed = b;
    }

    bool seed() const
    {
      return _seed;
    }
  
    uint64_t numberOfTcs() const {
      return numE;
    }

    uint64_t totE() const {
      return sumE;
    }

    uint64_t ceeE() const
    {
      return sumCee;
    }

    uint64_t ceeECore() const
    {
      return sumCeeCore;
    }

    uint64_t ceHEarly() const
    {
      return sumCehEarly;
    }

    double avgX() const
    {
      return (numX == 0.0 ? 0.0 : sumX / numX);
    }

    double avgY() const
    {
      return (numY == 0.0 ? 0.0 : sumY / numY);
    }

    unsigned sumW() const { return sumW_; }

    unsigned sumZ() const { return sumZ_; }

    uint64_t layerBits() const { return layerBits_; }

    int calcPhi() const
    {
      return atan2( avgY(), avgX()) * 720 / acos(-1.0);
    }

    unsigned int calcEta() const
    {
      return asinh(1.0 / sqrt(avgY() * avgY() + avgX() * avgX())) * 720 / acos(-1.0) - 256;     
    }

    void zero()
    {
      numE = 0;
      sumE = 0;
      ssqE = 0;
      numCee = 0;
      sumCee = 0;
      ssqCee = 0;
      sumCeeCore = 0;
      sumCehEarly = 0;
      layerBits_ = 0;
      sumW_ = 0.0;
      numX = 0.0;
      sumX = 0.0;
      ssqX = 0.0;
      numY = 0.0;
      sumY = 0.0;
      ssqY = 0.0;
      sumZ_ = 0.0;
      ssqZ_ = 0.0;
      _seed = false;
    }

    bool isZero() const
    {
      return numE == 0;
    }

    void addNN(const TcAccumulator *tca)
    {
      assert(vNN.size() < 6);
      vNN.push_back(tca);
    }

    bool isLocalMaximum() const
    {
      assert(vNN.size() > 0);

      const uint64_t e(totE());
      if (e == 0)
        return false;

      for (unsigned i(0); i < vNN.size(); i++)
      {
        if (vNN[i]->totE() >= e)
          return false;
      }
      return true;
    }

    void print() const
    {
      std::cout << "TcAccumulator"
                << " E = " << totE()
                << " CE-E E = " << ceeE()
                << " x = " << avgX()
                << " y = " << avgY()
                << " seed = " << (_seed ? "True" : "False")
                << std::endl;
    }

  private:
    uint64_t numE, sumE, ssqE;
    uint64_t numCee, sumCee, ssqCee;
    uint64_t sumCeeCore, sumCehEarly;
    uint64_t layerBits_;
    double sumW_, numX, sumX, ssqX, numY, sumY, ssqY, sumZ_, ssqZ_;
    bool _seed;

    std::vector<const TcAccumulator *> vNN;
  };

  double distanceXX(const double *ca, const double *cb)
  {
    // std::cout << "Distance of " << ca[0] << ", " << ca[1]
    //	      << " from " << cb[0] << ", " << cb[1];

    double d(sqrt((ca[0] - cb[0]) * (ca[0] - cb[0]) +
                  (ca[1] - cb[1]) * (ca[1] - cb[1])));

    // std::cout << " = " << d << std::flush << std::endl;
    return d;
  }

  template <unsigned nBins>
  class CentreArray
  {
  public:
    unsigned numberOfBins() const
    {
      return nBins;
    }

    double centre[3][nBins][nBins][2];
  };

  template <unsigned nBins>
  class TcAccumulatorArray
  {
  public:
    TcAccumulator vTca[3][nBins][nBins];

    void zero()
    {
      for (unsigned c(0); c < 3; c++)
      {
        for (unsigned i(0); i < nBins; i++)
        {
          for (unsigned j(0); j < nBins; j++)
          {
            vTca[c][i][j].zero();
          }
        }
      }
    }
  };

  class Stage2
  {
  public:
    static uint32_t unpack4E4MToUnsigned(uint8_t flt)
    {
      uint32_t e(flt >> 4);
      uint32_t m(flt & 0xf);

      if (e == 0)
        return m;
      else if (e == 1)
        return 16 + m;
      else
        return (32 + 2 * m + 1) << (e - 2);
    }

    enum
    {
      _nBins = 49
    };

    Stage2()
    {

      _ca = new CentreArray<_nBins>;

      // Set up clustering array (here hexagons)
      for (unsigned i(0); i < _nBins; i++)
      {
        int x(int(i) - (int(_nBins) / 2));
        for (unsigned j(0); j < _nBins; j++)
        {
          int y(int(j) - (int(_nBins) / 2));

          //_ca->centre[0][i][j][0]=((j%2)==1?x:x+0.5)*_rOverZ;
          //_ca->centre[0][i][j][1]=y*_rOverZ*sqrt(3.0)/2.0-0.5*_rOverZ/sqrt(3.0);
          _ca->centre[0][i][j][0] = ((j % 2) == 0 ? x : x + 0.5) * _rOverZ;
          _ca->centre[0][i][j][1] = y * _rOverZ * sqrt(3.0) / 2.0;

          _ca->centre[1][i][j][0] = _ca->centre[0][i][j][0];
          _ca->centre[1][i][j][1] = _ca->centre[0][i][j][1] + _rOverZ / sqrt(3.0);

          _ca->centre[2][i][j][0] = _ca->centre[0][i][j][0];
          _ca->centre[2][i][j][1] = _ca->centre[0][i][j][1] - _rOverZ / sqrt(3.0);
        }
      }

      _tcaa = new TcAccumulatorArray<_nBins>;

      // Set up NN associations
      for (unsigned i(0); i < _nBins; i++)
      {
        for (unsigned j(0); j < _nBins; j++)
        {
          for (unsigned c(0); c < 3; c++)
          {

            for (int i2(int(i) - 2); i2 <= int(i) + 2; i2++)
            {
              if (i2 >= 0 && i2 < _nBins)
              {
                for (int j2(int(j) - 2); j2 <= int(j) + 2; j2++)
                {
                  if (j2 >= 0 && j2 < _nBins)
                  {
                    if (distanceXX(_ca->centre[c][i][j], _ca->centre[(c + 1) % 3][i2][j2]) < 0.6 * _rOverZ)
                    {
                      // std::cout << "Match i,j,c,i2,j2,c2 = " << i << ", " << j << ", " << c
                      //	      << ", " << i2 << ", " << j2 << ", " << (c+1)%3 << std::endl;
                      _tcaa->vTca[c][i][j].addNN(&(_tcaa->vTca[(c + 1) % 3][i2][j2]));
                    }
                    if (distanceXX(_ca->centre[c][i][j], _ca->centre[(c + 2) % 3][i2][j2]) < 0.6 * _rOverZ)
                    {
                      // std::cout << "Match i,j,c,i2,j2,c2 = " << i << ", " << j << ", " << c
                      //	      << ", " << i2 << ", " << j2 << ", " << (c+2)%3 << std::endl;
                      _tcaa->vTca[c][i][j].addNN(&(_tcaa->vTca[(c + 2) % 3][i2][j2]));
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    void run(const std::vector<TPGTriggerCellWord> &vTcw,
             std::vector<TPGClusterData> &vCld)
    {
      std::vector<l1thgcfirmware::HGCalCluster_HW> vCldCMSSW_dummy;
      run(vTcw, vCld, vCldCMSSW_dummy);
    }

    void run(const std::vector<TPGTriggerCellWord> &vTcw,
             std::vector<TPGClusterData> &vCld,
             std::vector<l1thgcfirmware::HGCalCluster_HW> &vCldCMSSW)
    {

      // Accumulate

      _tcaa->zero();

      double dr2Limit(_rOverZ * _rOverZ / 3.0);

      for (unsigned itc(0); itc < vTcw.size(); itc++)
      {
        TPGTriggerCellFloats vTcf(vTcw[itc]);
        float tcfX(vTcf.getXOverZF());
        float tcfY(vTcf.getYOverZF());

        for (unsigned c(0); c < 3; c++)
        {
          double dr2Min(1.0e10);
          unsigned iMin, jMin;

          for (unsigned i(0); i < _nBins; i++)
          {
            for (unsigned j(0); j < _nBins; j++)
            {
              double dx(tcfX - _ca->centre[c][i][j][0]);
              double dy(tcfY - _ca->centre[c][i][j][1]);
              double dr2(dx * dx + dy * dy);

              if (dr2Min > dr2)
              {
                dr2Min = dr2;
                iMin = i;
                jMin = j;
              }
            }
          }

          if (dr2Min > dr2Limit)
          {
            std::cout << "Accumulation: at i,j = " << iMin << ", " << jMin
                      << ", dr2Min = " << dr2Min << " > " << dr2Limit
                      << std::endl;
            assert(false);
          }

          _tcaa->vTca[c][iMin][jMin].accumulate(vTcf);
        }
      }

      // Find local maxima

      vCld.resize(0);

      for (unsigned c(0); c < 3; c++)
      {
        for (unsigned i(0); i < _nBins; i++)
        {
          for (unsigned j(0); j < _nBins; j++)
          {
            double phiNorm(6.0 * atan2(_ca->centre[c][i][j][1], _ca->centre[c][i][j][0]) / acos(-1));

            if (phiNorm >= -2.0 && phiNorm < 2.0 && _tcaa->vTca[c][i][j].isLocalMaximum())
            {
              TPGClusterData tcd;
              tcd.setEnergy(_tcaa->vTca[c][i][j].totE() / 64);
              if (_tcaa->vTca[c][i][j].ceeE() >= _tcaa->vTca[c][i][j].totE())
              {
                tcd.setCeeFraction(255);
              }
              else
              {
                tcd.setCeeFraction((256 * _tcaa->vTca[c][i][j].ceeE()) / _tcaa->vTca[c][i][j].totE());
              }
              tcd.setPhi(_tcaa->vTca[c][i][j].calcPhi());
              tcd.setEta(_tcaa->vTca[c][i][j].calcEta());
      	      tcd.setNumberOfTcs(_tcaa->vTca[c][i][j].numberOfTcs());

              vCld.push_back(tcd);

              // Also calculate properties based on CMSSW/emulator code
              l1thgcfirmware::HGCalCluster_HW hwCluster;
              clusterProperties( _tcaa->vTca[c][i][j], hwCluster);
              // std::cout << "Got a hwCluster : " << hwCluster.e << " " << hwCluster.pack()[0] << std::endl;
              vCldCMSSW.emplace_back(hwCluster);
            }
          }
        }
      }

      /* DISABLE FOR NOW
      // TEMP: use TCs to fill towers
      std::memset(_towerData,0,2*20*24*sizeof(uint16_t));
      
      for(unsigned itc(0);itc<vTcw.size();itc++) {
	TPGTriggerCellFloats vTcf(vTcw[itc]);
	double phiDegrees(60.0+180.0*atan2(vTcf.getYOverZF(),vTcf.getXOverZF())/acos(-1.0));
	if(phiDegrees>=0.0 && phiDegrees<120.0) {
	  unsigned f(phiDegrees/5.0);
	  assert(f>=0 && f<24);
	  
	  double rho=sqrt(vTcf.getXOverZF()*vTcf.getXOverZF()+vTcf.getYOverZF()*vTcf.getYOverZF());
	  double etaBins(72.0*(asinh(1.0/rho)-3.0)/acos(-1))+20.0);
	  if(etaBins>=0.0 && etaBins<20.0) {
	    unsigned h(etaBins);
	    assert(h>=0 && h<20);
	    
	    unsigned e(vTcf.getEnergy());
	    if(vTcf.getLayer()<=26) _towerData[0][h][f]+=e;
	    else                    _towerData[1][h][f]+=e;
	  }
	}
      }
      
      for(unsigned eta(0);eta<20;eta++) {
	for(unsigned phi(0);phi<24;phi++) {
	  uint32_t total(_towerData[0][eta][phi]+_towerData[1][eta][phi]);
	  if(total>0xffff) total=0xffff;
	  
	  unsigned fraction(7);
	  if(_towerData[0][eta][phi]>0) fraction=(8*_towerData[1][eta][phi])/_towerData[0][eta][phi];
	  if(fraction>7) fraction=7;
	  _towerOutput[eta][phi]=total>>6|fraction<<10;
	}
      }
      */
    }

    void clusterProperties(const TcAccumulator& accumulatedCluster, l1thgcfirmware::HGCalCluster_HW& hwCluster) const 
    {
      using namespace l1thgcfirmware;

      hwCluster.e = Scales::HGCaltoL1_et(accumulatedCluster.totE());
      hwCluster.e_em = Scales::HGCaltoL1_et(accumulatedCluster.ceeE());
      hwCluster.fractionInCE_E = Scales::makeL1EFraction(accumulatedCluster.ceeE(), accumulatedCluster.totE());
      hwCluster.fractionInCoreCE_E = Scales::makeL1EFraction(accumulatedCluster.ceeECore(), accumulatedCluster.ceeE());
      hwCluster.fractionInEarlyCE_E = Scales::makeL1EFraction(accumulatedCluster.ceHEarly(), accumulatedCluster.totE());
      hwCluster.setGCTBits(); // Derived from energy and energy fractions that have just been set

      std::vector<int> layeroutput = showerLengthProperties(accumulatedCluster.layerBits());
      hwCluster.firstLayer = layeroutput[0];
      hwCluster.lastLayer = layeroutput[1];
      hwCluster.showerLength = layeroutput[2];
      hwCluster.coreShowerLength = layeroutput[3];
      hwCluster.nTC = accumulatedCluster.numberOfTcs();

      hwCluster.w_eta = accumulatedCluster.calcEta() + 256; // Previously, eta was calculated in the firmware in global coordinates (0->3), but now it's more local.  Add the offset here, as it's what is expected in the final packet
      hwCluster.w_phi = accumulatedCluster.calcPhi();
      bool saturatedPhi = false;  // Need to base this on calculated phi
      bool nominalPhi = false;  // Need to base this on calculated phi
      hwCluster.w_z = Scales::HGCaltoL1_z( float(accumulatedCluster.sumZ()) / accumulatedCluster.sumW() );

      // Quality flags are placeholders at the moment
      hwCluster.setQualityFlags(Scales::HGCaltoL1_et(accumulatedCluster.ceeECore()), Scales::HGCaltoL1_et(accumulatedCluster.ceHEarly()), 0, 0, /*c->sat_tc(), c->shapeq(),*/ saturatedPhi, nominalPhi);

    }

    std::vector<int> showerLengthProperties(unsigned long int layerBits) const {
      int counter = 0;
      int firstLayer = 0;
      bool firstLayerFound = false;
      int lastLayer = 0;
      std::vector<int> layerBits_array;

      std::bitset<34> layerBitsBitset(layerBits);
      for (size_t i = 0; i < layerBitsBitset.size(); ++i) {
          bool bit = layerBitsBitset[34-1-i];
          if ( bit ) {
            if ( !firstLayerFound ) {
              firstLayer = i + 1;
              firstLayerFound = true;
            }
            lastLayer = i+1;
            counter += 1;
          } else {
            layerBits_array.push_back(counter);
            counter = 0;
          }
      }

      int showerLen = lastLayer - firstLayer + 1;
      int coreShowerLen = 34;
      if (!layerBits_array.empty()) {
        coreShowerLen = *std::max_element(layerBits_array.begin(), layerBits_array.end());
      }
      return {firstLayer, lastLayer, showerLen, coreShowerLen};
    }

    const CentreArray<_nBins> &centreArray() const
    {
      return *_ca;
    }

    void run(const TPGBEDataformat::Stage1ToStage2DataArray &_s12,
             TPGBEDataformat::Stage2ToL1tDataArray &s2L1t)
    {

      // Unpack TCs into integer format stored internally
      // run(_s12,_vTriggerCellWord);

      // Call run to make clusters
      // void run(_vTriggerCellWord,_vClusterData);

      // Complete towers from partials
      // run(_s12);

      // Combine clusters and towers to make total packet
      // run(_vClusterData,s2L1t);
    }

    void run(const std::vector<TPGBEDataformat::Stage1ToStage2Data *> &vS12)
    {
      std::memset(_towerData, 0, 2 * 20 * 24 * sizeof(uint16_t));

      for (unsigned l(0); l < vS12.size(); l++)
      {
        unsigned offset(5 * (l % 6));
        if ((l % 6) >= 4)
          offset -= 6;

        for (unsigned w(0); w < 100; w++)
        {
          for (unsigned eh(0); eh < 2; eh++)
          {
            uint8_t e(vS12[l]->getPTT(w, eh));
            uint32_t eunpacked(unpack4E4MToUnsigned(e));
            _towerData[eh][w / 5][offset + w % 5] += eunpacked;
          }
        }
      }

      for(unsigned eta(0);eta<20;eta++) {
	for(unsigned phi(0);phi<24;phi++) {
	  uint32_t total(_towerData[0][eta][phi]+_towerData[1][eta][phi]);
	  if(total>0xffff) total=0xffff;

	  unsigned fraction(7);
	  if(_towerData[0][eta][phi]>0) fraction=(8*_towerData[1][eta][phi])/_towerData[0][eta][phi];
	  if(fraction>7) fraction=7;
	  _towerOutput[eta][phi]=total>>6|fraction<<10;
	}
      }
    }
    
    void run(const std::vector<TPGBEDataformat::Stage1ToStage2Data> &vS12){
      std::memset(_towerData, 0, 2 * 20 * 24 * sizeof(uint16_t));
      
      for (unsigned l(0); l < vS12.size(); l++){
        unsigned offset(5 * (l % 6));
        if ((l % 6) >= 4)
          offset -= 6;

        for (unsigned w(0); w < 100; w++){
          for (unsigned eh(0); eh < 2; eh++){
            uint8_t e(vS12[l].getPTT(w, eh));
            uint32_t eunpacked(unpack4E4MToUnsigned(e));
            _towerData[eh][w / 5][offset + w % 5] += eunpacked;
          }
        }
      }
      
      for(unsigned eta(0);eta<20;eta++) {
	for(unsigned phi(0);phi<24;phi++) {
	  uint32_t total(_towerData[0][eta][phi]+_towerData[1][eta][phi]);
	  if(total>0xffff) total=0xffff;

	  unsigned fraction(7);
	  if(_towerData[0][eta][phi]>0) fraction=(8*_towerData[1][eta][phi])/_towerData[0][eta][phi];
	  if(fraction>7) fraction=7;
	  _towerOutput[eta][phi]=total>>6|fraction<<10;
	}
      }
    }
    
    uint32_t getTowerData(int iz, int ieta, int iphi) const { return _towerData[iz][ieta][iphi];}
    uint16_t getTowerOutput(int ieta, int iphi) const { return _towerOutput[ieta][iphi];}
    
  private:
    // static const unsigned _nBins;
    static const double _rOverZ;
    
    std::vector<TPGTriggerCellWord> _vTriggerCellWord;

    uint32_t _towerData[2][20][24];
    uint16_t _towerOutput[20][24];
    
    std::vector<TPGClusterData> _vClusterData;

    CentreArray<_nBins> *_ca;
    TcAccumulatorArray<_nBins> *_tcaa;
  };

  // const double Stage2::_rOverZ(0.032);
  const double Stage2::_rOverZ(0.016 * sqrt(3.0));
}
#endif
