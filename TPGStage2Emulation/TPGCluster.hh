/**********************************************************************
 Created on : 22/04/2025
 Purpose    : HGCAL TPG Cluster information
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#ifndef TPGCluster_h
#define TPGCluster_h

#include "TPGLSBScales.hh"
#include "TPGBEDataformat.hh"
#include "CMSSWCode/DataFormats/L1THGCal/interface/HGCalCluster_HW.h"

//Class to access cluster information
class TPGCluster{
public:
  TPGCluster() {}
  TPGCluster(const l1thgcfirmware::HGCalCluster_HW *clus) : cldata(*clus) {}
  void setHGCalCluster_HW(const l1thgcfirmware::HGCalCluster_HW *clus) { cldata = *clus ; }

  uint32_t getEnergy() const { return cldata.e.to_uint(); }
  double getEnergyGeV() const { return getEnergy() * lsbScales.LSB_E(); }
  double getCeeFractionF() const { return cldata.fractionInCE_E.to_uint() * lsbScales.LSB_Frac(); }

  int getLocalPhi() const { return cldata.w_phi.to_int(); }
  double getLocalPhiRad() const { return getLocalPhi() * lsbScales.LSB_phi(); }
  
  double getGlobalPhiRad(unsigned s) const {    
    int32_t gPhi;
    if(s<3) gPhi= 4*90+4*120*s    +getLocalPhi();
    else    gPhi= 4*90-4*120*(s-3)-getLocalPhi();
    
    if(gPhi<-720) gPhi+=2*720;
    if(gPhi>=720) gPhi-=2*720;
    return gPhi*lsbScales.LSB_phi();
  }

  uint32_t getEta() const { return cldata.w_eta.to_uint(); }
  double getEtaRad() const { return getEta() * lsbScales.LSB_eta(); } 
  double getGlobalEtaRad(unsigned s) const { return (s<3) ? (-getEtaRad()) : getEtaRad() ;  }

  double getLocalXOverZF() const { return cos(getLocalPhiRad())/sinh(getEtaRad()); }
  double getLocalYOverZF() const { return sin(getLocalPhiRad())/sinh(getEtaRad()); }
  double getGlobalXOverZF(unsigned s) const { return cos(getGlobalPhiRad(s))/sinh(getGlobalEtaRad(s)); }
  double getGlobalYOverZF(unsigned s) const { return sin(getGlobalPhiRad(s))/sinh(getGlobalEtaRad(s)); }
  
  uint32_t getZ() const { return cldata.w_z.to_uint(); }
  double getZCm() const { return (lsbScales.abs_z_global_min_cm() + getZ() * lsbScales.LSB_z()/10.); }
  double getGlobalZCm(unsigned s) const { return (s<3) ? (-getZCm()) : getZCm() ; }
  
  double getRhoOverZF() const { return 1.0/sinh(getEtaRad()); }
  double getGlobalRhoOverZF(unsigned s) const { return 1.0/sinh(getGlobalEtaRad(s)); }

private:
  l1thgcfirmware::HGCalCluster_HW cldata;
  TPGLSBScales::TPGStage2ClusterLSB lsbScales;
};

// double TPGClusterFloats::_eScale=4.0;
// double TPGClusterFloats::_ceeScale=256.0;
// double TPGClusterFloats::_phiScale=720.0/acos(-1);
// double TPGClusterFloats::_etaScale=720.0/acos(-1);
// double TPGClusterFloats::_etaOffset=256.0;
// double TPGClusterFloats::_zScale=20.0;
// double TPGClusterFloats::_zOffset=322.155;

#endif
