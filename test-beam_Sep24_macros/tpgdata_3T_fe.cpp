/**********************************************************************
 Created on : 18/10/2024
 Purpose    : Study of TPG data for 3 silicon trains, 1 motherboard and external trigger
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>

#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TFile.h"

#include "TFileHandlerLocal.h"
#include "FileReader.h"

#include "yaml-cpp/yaml.h"
#include <deque>

#include <string>
#include <fstream>
#include <cassert>
#include <bitset>

#include "TPGFEDataformat.hh"
#include "TPGBEDataformat.hh"
#include "TPGFEConfiguration.hh"
#include "TPGFEModuleEmulation.hh"
#include "TPGFEReader3TSep2024.hh"
#include "Stage1IO.hh"

int main(int argc, char** argv)
{
  
  std::cout << "========= tpgdata_3T_fe ===============" << std::endl;
  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return false;
  }
  
  uint64_t getTotalNofEvents(uint32_t, uint32_t);
  //===============================================================================================================================
  //Assign relay,run and numbers of events from input
  //===============================================================================================================================
  uint32_t relayNumber(0);
  uint32_t runNumber(0);
  uint32_t linkNumber(0);
  uint64_t totEvents(0);
  uint64_t nofEvents(1);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  //Notes:
  // 1. firmware test for third train 1727007636 (old ECON-T configuration so should not be analyzed)
  // 2. first pedestal with 3rd layer in configuration 1727033054 and installed 1727099251 (with correct ECON-T configuration)
  // 3. first technical with third layer : 1727099443 //https://cmsonline.cern.ch/webcenter/portal/cmsonline/Common/Elog?__adfpwp_action_portlet=683379043&__adfpwp_backurl=https%3A%2F%2Fcmsonline.cern.ch%3A443%2Fwebcenter%2Fportal%2Fcmsonline%2FCommon%2FElog%3F_afrRedirect%3D23225796808764440&_piref683379043.strutsAction=%2FviewMessageDetails.do%3FmsgId%3D1237840
  // 4. swap back roc2 config of mod16 for relays after 1727116899 (not necessary for this code, kept as bookkeeping)
  // 5. List of run flagged as green by Paul for TC Processor are Relay1727219172 to Run1727219175 and probably Relay1727211141
  // 6. Sampling phase issue has been solved for relays between 1727123194 to 1727153189
  // 7. 14 0xcafecafe separators from Relay 1727191081 onwards
  
  if(relayNumber<1727099251){
    std::cerr << "Third layer is not properly configured" << std::endl;
    return false;
  }
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  if(argc = 4){
    std::istringstream issnEvents(argv[3]);
    issnEvents >> nofEvents;
  }
  // totEvents = getTotalNofEvents(relayNumber, runNumber);
  // if(nofEvents>totEvents){
  //   nofEvents = totEvents ;
  //   std::cout << "Setting number of events to process to " << nofEvents << std::endl;
  // }
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Read channel mapping
  //===============================================================================================================================
  TPGFEConfiguration::Configuration cfgs;
  cfgs.setSiChMapFile("cfgmap/WaferCellMapTraces.txt");
  cfgs.setSciChMapFile("cfgmap/channels_sipmontile_HDtypes.hgcal.txt");
  cfgs.initId();
  cfgs.readSiChMapping();
  cfgs.readSciChMapping();
  cfgs.loadModIdxToNameMapping();
  cfgs.loadMuxMapping();
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Read ECON-T config : note that we are not reading any ECON-D config
  //===============================================================================================================================
  //First ready a dummy config
  uint32_t zside = 0, sector = 0, link = 0, det = 0;
  uint32_t econt = 0, selTC4 = 1, module = 0;
  const uint32_t noflpGBTs = 4;
  const uint32_t nofECONs = 3; //actually less for two links, see below
  TPGFEConfiguration::TPGFEIdPacking pck;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons ;iecont++){
      uint32_t idx = pck.packModId(zside, sector, ilink, det, iecont, selTC4, module); 
      cfgs.setModulePath(zside, sector, ilink, det, iecont, selTC4, module);
      cfgs.setEconTFile("cfgmap/init_econt.yaml"); //read a dummy econt since the original was not available earlier
      cfgs.readEconTConfigYaml();
    }//econt loop
  }//lpGBT loop

  //Step2: update all ECON-T configs corresponding to test-beam after the inclusion of third train
  link = 0; econt = 0; uint32_t lp0_bc0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 0; econt = 1; uint32_t lp0_bc1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 0; econt = 2; uint32_t lp0_bc2 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 0; uint32_t lp1_stc160 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 1; uint32_t lp1_stc161 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 2; uint32_t lp1_stc162 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 2; econt = 0; uint32_t lp2_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 2; econt = 1; uint32_t lp2_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 3; econt = 0; uint32_t lp3_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 3; econt = 1; uint32_t lp3_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  
  uint32_t density(0);
  uint32_t droplsb(1);
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar =  cfgs.getEconTPara();
  for(const auto& it : econTPar){
    econTPar[it.first].setDensity(density);
    econTPar[it.first].setDropLSB(droplsb);
    
    if(it.first==lp0_bc0){              //// starting 1st train
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(3);      
    }else if (it.first==lp0_bc1){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(2);     
    }else if (it.first==lp0_bc2){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(2);
    }else if (it.first==lp1_stc160){    //// starting 2nd train
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp1_stc161){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp1_stc162){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp2_stc4a0){    //// starting 3rd train
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(4);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp2_stc4a1){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp3_stc4a0){    //// starting motherboard
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(4);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp3_stc4a1){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(3);
      econTPar[it.first].setSTCType(3);
    }
    std::cout << "Modtype " << it.first
	      << ", getOuttype: " << econTPar[it.first].getOutType()
	      << ", select: " << econTPar[it.first].getSelect()
      	      << ", stctype: " << econTPar[it.first].getSTCType()
	      << ", typename: " << TPGFEDataformat::tctypeName[econTPar[it.first].getOutType()]
	      << std::endl;
    econTPar[it.first].print();
  }//econt loop
  //===============================================================================================================================
  
  // ===============================================================================================================================
  // Set and Initialize the ECONT reader
  // ===============================================================================================================================
  TPGFEReader::ECONTReader econTReader(cfgs);
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>>> econtarray; //event,moduleId (from link)
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::TrigTCProcData>>> tcprocarray; //event,moduleId (from link)
  econTReader.setNofCAFESep(11);
  if(relayNumber>=1727191081) econTReader.setNofCAFESep(14);
  // ===============================================================================================================================
  
  //===============================================================================================================================
  //Process certain refernce eventlist
  //===============================================================================================================================
  /*TcTp1 events */ uint64_t refEvt[3] = {17249, 17253,  17288};
  std::vector<uint64_t> refEvents ;
  for(int ievent=0;ievent<3;ievent++) refEvents.push_back(refEvt[ievent]);
  //when the following line is "commented" only the events filled in "refEvents" will be processed instead of nofevents set in the command line
  refEvents.resize(0);
  //===============================================================================================================================
  
  uint64_t minEventTPG, maxEventTPG;  
  long double nloopEvent =  100000;
  int nloop = TMath::CeilNint(nofEvents/nloopEvent) ;
  if(refEvents.size()>0) nloop = 1;
  std::cout<<"nloop: "<<nloop<<std::endl;
  uint64_t processed_events = 0;
  
  bool printCondn = false;
  bool allTCszero[noflpGBTs][nofECONs];
  uint32_t nofZeroBxs[noflpGBTs][nofECONs];
  std::vector<uint64_t> NofAnyZeroEvents[noflpGBTs][nofECONs];
  std::vector<uint64_t> NofAllZeroEvents[noflpGBTs][nofECONs];
  std::vector<uint64_t> NofS1EmulDataMM[noflpGBTs][nofECONs];
  std::vector<uint64_t> NofFixedPatEvents[noflpGBTs][nofECONs];
  std::vector<uint64_t> NofAllSameElinksEvents[noflpGBTs][nofECONs];
  std::vector<uint64_t> NofDuplicateElinksEvents[noflpGBTs][nofECONs];
  std::vector<uint64_t> NofAnyElZeroEvents[noflpGBTs][nofECONs];
  std::vector<uint64_t> NofAllElZeroEvents[noflpGBTs][nofECONs];
  std::vector<uint64_t> NofAllElDupEvents[noflpGBTs][nofECONs];
  std::vector<uint64_t> NofGoodEvents[noflpGBTs][nofECONs];
  
  std::vector<uint32_t> nums;
  
  uint32_t scan_pattern = 0xffaaffaa;
  
  uint32_t nofpatmatch(0);
  uint32_t nofDuplicates(0);
  uint32_t nofelzero_anybx(0);
  uint32_t nofelzero_allbx(0);
  uint32_t nofdupel_allbx(0);
  bool isGood(false);
  
  for(int ieloop=0;ieloop<nloop;ieloop++){
    
    minEventTPG = ieloop*nloopEvent ;
    maxEventTPG = (ieloop+1)*nloopEvent;
    maxEventTPG = (maxEventTPG>uint64_t(nofEvents)) ? uint64_t(nofEvents) : maxEventTPG ;
    printf("iloop : %d, minEventTPG = %lu, maxEventTPG = %lu\n",ieloop,minEventTPG, maxEventTPG);
    std::cerr<<"iloop : "<< ieloop <<", minEventTPG = "<< minEventTPG <<", maxEventTPG = " << maxEventTPG << std::endl;
    
    econtarray.clear();
    tcprocarray.clear();
    
    econTReader.init(relayNumber,runNumber,0);
    std::cout<<"TRIG Before: elink and unpacker size : " << econtarray.size() << ", tcprocarray.size(): " << tcprocarray.size() <<std::endl;
    econTReader.getEvents(refEvents, minEventTPG, maxEventTPG, econtarray, tcprocarray);
    std::cout<<"TRIG After:  elink and unpacker size : " << econtarray.size() << ", tcprocarray.size(): " << tcprocarray.size() <<std::endl;
    econTReader.terminate();
    
    for(const auto& it :  econtarray){ //this is essencially event loop
      uint64_t event = it.first ;
      if(printCondn) std::cout << "Processing event: " << event << std::endl;
      for(int ilink=0;ilink<noflpGBTs;ilink++){
	int maxecons = (ilink<=1)?3:2;
	for(int iecont=0;iecont<maxecons;iecont++){
	  uint32_t moduleId = pck.packModId(zside, sector, ilink, det, iecont, selTC4, module); 
	  std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>> econtdata =  econtarray[event];	  
	  TPGBEDataformat::UnpackerOutputStreamPair upemul,updata;
	  TPGFEDataformat::TcRawDataPacket vTCel;	
	  TPGBEDataformat::Trig24Data trdata;
	  
	  allTCszero[ilink][iecont] = false;
	  nofZeroBxs[ilink][iecont] = 0;          
	  nofelzero_allbx = 0;
	  nofdupel_allbx = 0;
	  isGood = true;
	  
	  for(const auto& econtit : econtdata){
	    if(econtit.first!=moduleId) continue;
	    trdata = econtit.second ;
	    const uint32_t *refel = trdata.getElinks(0); 
	    if(printCondn) std::cout << "Dataloop:: event: " << event << ", moduleId : " << econtit.first << std::endl;
	    for(int ibx=0;ibx<7;ibx++){
	      //for(int ibx=3;ibx<4;ibx++){
	      const uint32_t *el = trdata.getElinks(ibx); 
	      uint32_t bx_2 = (el[0]>>28) & 0xF;
	      //// =========== Check the elinks if they are matching predefined paterrn, whether they are identical elinks ============
	      nofpatmatch = 0 ;
	      nofDuplicates = 0 ;
	      nofelzero_anybx = 0 ;
	      nums.clear();
	      bool isDup = true;
	      for(int iel = 0 ; iel < econTPar[moduleId].getNElinks() ; iel++){
		if((el[iel]&0xff)==(scan_pattern&0xff) and ((el[iel]>>16)&0xff)==((scan_pattern>>16)&0xff)) nofpatmatch++;
		if(el[iel]==0) nofelzero_anybx++;
		if(std::find(nums.begin(),nums.end(),el[iel])==nums.end())
		  nums.push_back(el[iel]);
		else
		  nofDuplicates++;
		if(el[iel]!=refel[iel]) isDup = false;
	      }	     
	      bool isAllSame = (((nofDuplicates+1)==econTPar[moduleId].getNElinks()) or (nofelzero_anybx==econTPar[moduleId].getNElinks()))?true:false;
	      
	      if(printCondn){
		std::cout << "========== ibx : " << ibx << " ==========" << std::endl;
		for(int iel = 0 ; iel < econTPar[moduleId].getNElinks() ; iel++) std::cout << "iel: " << iel << std::hex << " elink: 0x" << el[iel]  << std::dec << std::endl;
		std::cout <<"Nof elinks with zeros: " << nofelzero_anybx <<", nofpatmatch : " << nofpatmatch << ", isAllSame: " << isAllSame << ", nofDuplicates: " << nofDuplicates << std::endl;
	      }
	      if(nofelzero_anybx==econTPar[moduleId].getNElinks()) {nofelzero_allbx++; isGood = false;}  
	      if(nofelzero_anybx>0) NofAnyElZeroEvents[ilink][iecont].push_back(event);
	      if(isAllSame) {NofAllSameElinksEvents[ilink][iecont].push_back(event); isGood = false;}  
	      if(nofDuplicates>=2) {NofDuplicateElinksEvents[ilink][iecont].push_back(event); isGood = false;}  
	      if(nofpatmatch>0) {NofFixedPatEvents[ilink][iecont].push_back(event);  isGood = false;}  
	      if(isDup) nofdupel_allbx++;
	      if(nofpatmatch==econTPar[moduleId].getNElinks()) continue;
	      //// =========== Elink checking is complete ===============
	      
	      if(ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, econTPar[moduleId].getNofTCs(), el, vTCel);
	      if(ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, econTPar[moduleId].getNofSTCs(), el, vTCel);
	      if(ilink==2 or ilink==3) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, econTPar[moduleId].getNofSTCs(), el, vTCel);	      
	      TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(bx_2, vTCel, upemul);
	      trdata.getUnpkStream(ibx,updata);
	      if(printCondn){		
		std::cout << "========== TC as read from elink for ibx : " << ibx << " ==========" << std::endl;
		vTCel.print();
		std::cout << "========== Emulated Stage1 unpacker output stream from TC for ibx : " << ibx << " ==========" << std::endl;
		upemul.print();
		std::cout << "========== TPG data: Stage1 unpacker output stream from firmware output for ibx : " << ibx << " ==========" << std::endl;	    
		updata.print();
		std::cout << "========== end of TPG data for ibx : " << ibx << " ==========" << std::endl;
	      }
	      
	      if((ilink==0 and !updata.isEqualBC(upemul)) or (ilink>0 and !updata.isEqualSTC(upemul)))
		NofS1EmulDataMM[ilink][iecont].push_back(event);
	      
	      uint32_t maxzeroenergy;
	      if(ilink==0) maxzeroenergy = 0;
	      if(ilink==1) maxzeroenergy = 16*econTPar[moduleId].getNofSTCs();
	      if(ilink==2 or ilink==3) maxzeroenergy = 2*econTPar[moduleId].getNofSTCs();
	      uint32_t totenergy = 0;
	      for(int itc=0;itc<vTCel.size();itc++) totenergy += vTCel.getTc(itc).energy();
	      if(totenergy==maxzeroenergy) {
		allTCszero[ilink][iecont] = true;
		nofZeroBxs[ilink][iecont]++;
	      }
	      
	    }//bx loop
	    
	    //if(eventCondn) trdata.print();
	  }//loop over econt data for a given run
	  
	  if(nofelzero_allbx==7) NofAllElZeroEvents[ilink][iecont].push_back(event);
	  if(nofdupel_allbx==7) NofAllElDupEvents[ilink][iecont].push_back(event);
	  if(nofZeroBxs[ilink][iecont]==7 and allTCszero[ilink][iecont] and nofelzero_allbx<7) NofAllZeroEvents[ilink][iecont].push_back(event);
	  if(nofZeroBxs[ilink][iecont]>0 and allTCszero[ilink][iecont] and nofelzero_allbx<7) NofAnyZeroEvents[ilink][iecont].push_back(event);
	  if(isGood and nofdupel_allbx<=6 and !allTCszero[ilink][iecont]) NofGoodEvents[ilink][iecont].push_back(event);
	  
	}//econt loop
      }//link loop
      processed_events++;
    }//this is essencially event loop
    
  }//ieloop

  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){
      std::sort(NofAnyZeroEvents[ilink][iecont].begin(), NofAnyZeroEvents[ilink][iecont].end());
      auto it = std::unique(NofAnyZeroEvents[ilink][iecont].begin(), NofAnyZeroEvents[ilink][iecont].end());
      NofAnyZeroEvents[ilink][iecont].erase(it, NofAnyZeroEvents[ilink][iecont].end());
      
      std::sort(NofAllZeroEvents[ilink][iecont].begin(), NofAllZeroEvents[ilink][iecont].end());
      auto it1 = std::unique(NofAllZeroEvents[ilink][iecont].begin(), NofAllZeroEvents[ilink][iecont].end());
      NofAllZeroEvents[ilink][iecont].erase(it1, NofAllZeroEvents[ilink][iecont].end());
      
      std::sort(NofS1EmulDataMM[ilink][iecont].begin(), NofS1EmulDataMM[ilink][iecont].end());
      auto it2 = std::unique(NofS1EmulDataMM[ilink][iecont].begin(), NofS1EmulDataMM[ilink][iecont].end());
      NofS1EmulDataMM[ilink][iecont].erase(it2, NofS1EmulDataMM[ilink][iecont].end());
      
      std::sort(NofFixedPatEvents[ilink][iecont].begin(), NofFixedPatEvents[ilink][iecont].end());
      auto it3 = std::unique(NofFixedPatEvents[ilink][iecont].begin(), NofFixedPatEvents[ilink][iecont].end());
      NofFixedPatEvents[ilink][iecont].erase(it3, NofFixedPatEvents[ilink][iecont].end());
      
      std::sort(NofAllSameElinksEvents[ilink][iecont].begin(), NofAllSameElinksEvents[ilink][iecont].end());
      auto it4 = std::unique(NofAllSameElinksEvents[ilink][iecont].begin(), NofAllSameElinksEvents[ilink][iecont].end());
      NofAllSameElinksEvents[ilink][iecont].erase(it4, NofAllSameElinksEvents[ilink][iecont].end());

      std::sort(NofDuplicateElinksEvents[ilink][iecont].begin(), NofDuplicateElinksEvents[ilink][iecont].end());
      auto it5 = std::unique(NofDuplicateElinksEvents[ilink][iecont].begin(), NofDuplicateElinksEvents[ilink][iecont].end());
      NofDuplicateElinksEvents[ilink][iecont].erase(it5, NofDuplicateElinksEvents[ilink][iecont].end());

      std::sort(NofAnyElZeroEvents[ilink][iecont].begin(), NofAnyElZeroEvents[ilink][iecont].end());
      auto it6 = std::unique(NofAnyElZeroEvents[ilink][iecont].begin(), NofAnyElZeroEvents[ilink][iecont].end());
      NofAnyElZeroEvents[ilink][iecont].erase(it6, NofAnyElZeroEvents[ilink][iecont].end());
      
      std::sort(NofAllElZeroEvents[ilink][iecont].begin(), NofAllElZeroEvents[ilink][iecont].end());
      auto it7 = std::unique(NofAllElZeroEvents[ilink][iecont].begin(), NofAllElZeroEvents[ilink][iecont].end());
      NofAllElZeroEvents[ilink][iecont].erase(it7, NofAllElZeroEvents[ilink][iecont].end());

      std::sort(NofAllElDupEvents[ilink][iecont].begin(), NofAllElDupEvents[ilink][iecont].end());
      auto it8 = std::unique(NofAllElDupEvents[ilink][iecont].begin(), NofAllElDupEvents[ilink][iecont].end());
      NofAllElDupEvents[ilink][iecont].erase(it8, NofAllElDupEvents[ilink][iecont].end());

      std::sort(NofGoodEvents[ilink][iecont].begin(), NofGoodEvents[ilink][iecont].end());
      auto it9 = std::unique(NofGoodEvents[ilink][iecont].begin(), NofGoodEvents[ilink][iecont].end());
      NofGoodEvents[ilink][iecont].erase(it9, NofGoodEvents[ilink][iecont].end());
      
    }//iecont
  }//ilink
  
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){
      if(NofAllZeroEvents[ilink][iecont].size()>0) std::cerr<< "/*all zero-energy events ilink:"<<ilink<<", iecont: "<<iecont<<"*/ uint64_t refEvt["<< NofAllZeroEvents[ilink][iecont].size() <<"] = {";
      // for(const uint64_t& totEvt : NofAllZeroEvents[ilink][iecont]) std::cerr<<totEvt << ", ";
      if(NofAllZeroEvents[ilink][iecont].size()>0) std::cerr<< "};" << std::endl;
    }
  }
  std::cerr<<std::endl;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){      
      if(NofAnyZeroEvents[ilink][iecont].size()>0) std::cerr<< "/*any zero-energy events ilink:"<<ilink<<", iecont: "<<iecont<<"*/ uint64_t refEvt["<< NofAnyZeroEvents[ilink][iecont].size() <<"] = {";
      // for(const uint64_t& totEvt : NofAnyZeroEvents[ilink][iecont]) std::cerr<<totEvt << ", ";
      if(NofAnyZeroEvents[ilink][iecont].size()>0) std::cerr<< "};" << std::endl;
    }
  }
  std::cerr<<std::endl;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){      
      if(NofS1EmulDataMM[ilink][iecont].size()>0) std::cerr<< "/*any S1EmulDataMM events ilink:"<<ilink<<", iecont: "<<iecont<<"*/ uint64_t refEvt["<< NofS1EmulDataMM[ilink][iecont].size() <<"] = {";
      // for(const uint64_t& totEvt : NofS1EmulDataMM[ilink][iecont]) std::cerr<<totEvt << ", ";
      if(NofS1EmulDataMM[ilink][iecont].size()>0) std::cerr<< "};" << std::endl;
    }
  }
  std::cerr<<std::endl;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){      
      if(NofFixedPatEvents[ilink][iecont].size()>0) std::cerr<< "/*any elinks of 7 bxs has fixed-pattern events ilink:"<<ilink<<", iecont: "<<iecont<<"*/ uint64_t refEvt["<< NofFixedPatEvents[ilink][iecont].size() <<"] = {";
      // for(const uint64_t& totEvt : NofFixedPatEvents[ilink][iecont]) std::cerr<<totEvt << ", ";
      if(NofFixedPatEvents[ilink][iecont].size()>0) std::cerr<< "};" << std::endl;
    }
  }
  std::cerr<<std::endl;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){      
      if(NofAllSameElinksEvents[ilink][iecont].size()>0) std::cerr<< "/*all duplicate elinks in any 7 bxs ilink:"<<ilink<<", iecont: "<<iecont<<"*/ uint64_t refEvt["<< NofAllSameElinksEvents[ilink][iecont].size() <<"] = {";
      // for(const uint64_t& totEvt : NofAllSameElinksEvents[ilink][iecont]) std::cerr<<totEvt << ", ";
      if(NofAllSameElinksEvents[ilink][iecont].size()>0) std::cerr<< "};" << std::endl;
    }
  }
  std::cerr<<std::endl;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){      
      if(NofDuplicateElinksEvents[ilink][iecont].size()>0) std::cerr<< "/*any duplicates elinks in any 7 bxs ilink:"<<ilink<<", iecont: "<<iecont<<"*/ uint64_t refEvt["<< NofDuplicateElinksEvents[ilink][iecont].size() <<"] = {";
      // for(const uint64_t& totEvt : NofDuplicateElinksEvents[ilink][iecont]) std::cerr<<totEvt << ", ";
      if(NofDuplicateElinksEvents[ilink][iecont].size()>0) std::cerr<< "};" << std::endl;
    }
  }
  std::cerr<<std::endl;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){
      if(NofAllElZeroEvents[ilink][iecont].size()>0) std::cerr<< "/*all elink zero events ilink:"<<ilink<<", iecont: "<<iecont<<"*/ uint64_t refEvt["<< NofAllElZeroEvents[ilink][iecont].size() <<"] = {";
      // for(const uint64_t& totEvt : NofAllElZeroEvents[ilink][iecont]) std::cerr<<totEvt << ", ";
      if(NofAllElZeroEvents[ilink][iecont].size()>0) std::cerr<< "};" << std::endl;
    }
  }
  std::cerr<<std::endl;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){      
      if(NofAnyElZeroEvents[ilink][iecont].size()>0) std::cerr<< "/*any elink zero events ilink:"<<ilink<<", iecont: "<<iecont<<"*/ uint64_t refEvt["<< NofAnyElZeroEvents[ilink][iecont].size() <<"] = {";
      // for(const uint64_t& totEvt : NofAnyElZeroEvents[ilink][iecont]) std::cerr<<totEvt << ", ";
      if(NofAnyElZeroEvents[ilink][iecont].size()>0) std::cerr<< "};" << std::endl;
    }
  }
  std::cerr<<std::endl;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){      
      if(NofAllElDupEvents[ilink][iecont].size()>0) std::cerr<< "/*all bx duplicates zero events ilink:"<<ilink<<", iecont: "<<iecont<<"*/ uint64_t refEvt["<< NofAllElDupEvents[ilink][iecont].size() <<"] = {";
      // for(const uint64_t& totEvt : NofAllElDupEvents[ilink][iecont]) std::cerr<<totEvt << ", ";
      if(NofAllElDupEvents[ilink][iecont].size()>0) std::cerr<< "};" << std::endl;
    }
  }
  std::cerr<<std::endl;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons;iecont++){      
      if(NofGoodEvents[ilink][iecont].size()>0) std::cerr<< "/*all bx duplicates zero events ilink:"<<ilink<<", iecont: "<<iecont<<"*/ uint64_t refEvt["<< NofGoodEvents[ilink][iecont].size() <<"] = {";
      // for(const uint64_t& totEvt : NofGoodEvents[ilink][iecont]) std::cerr<<totEvt << ", ";
      if(NofGoodEvents[ilink][iecont].size()>0) std::cerr<< "};" << std::endl;
    }
  }
  std::cerr<<std::endl;
  std::cerr << " RelayNo. " << "| "
	    << " RunN. " << "| "
	    << " maxEvent " << "| "
	    << " processed " << "| "
	    << " Mod-19 (allel 0) | "
    	    << " Mod-20 (allel 0) | "
    	    << " Mod-18 (allel 0) | "
	    << " Mod-16 (allel 0) | "
    	    << " Mod-17 (allel 0) | "
    	    << " Mod-14 (allel 0) | "
    	    << " Mod-15 (allel 0) | "
	    << " Mod-p (allel 0) | "
    	    << " MB-E0 (allel 0) | "
	    << " MB-E1 (allel 0) | "

	    << " Mod-19 (fixpat) | "
    	    << " Mod-20 (fixpat) | "
    	    << " Mod-18 (fixpat) | "
	    << " Mod-16 (fixpat) | "
    	    << " Mod-17 (fixpat) | "
    	    << " Mod-14 (fixpat) | "
    	    << " Mod-15 (fixpat) | "
	    << " Mod-p (fixpat) | "
    	    << " MB-E0 (fixpat) | "
	    << " MB-E1 (fixpat) | "

	    << " Mod-19 (allTC 0) | "
    	    << " Mod-20 (allTC 0) | "
    	    << " Mod-18 (allTC 0) | "
	    << " Mod-16 (allTC 0) | "
    	    << " Mod-17 (allTC 0) | "
    	    << " Mod-14 (allTC 0) | "
    	    << " Mod-15 (allTC 0) | "
	    << " Mod-p (allTC 0) | "
    	    << " MB-E0 (allTC 0) | "
	    << " MB-E1 (allTC 0) | "

	    << " Mod-19 (anyTC 0) | "
    	    << " Mod-20 (anyTC 0) | "
    	    << " Mod-18 (anyTC 0) | "
	    << " Mod-16 (anyTC 0) | "
    	    << " Mod-17 (anyTC 0) | "
    	    << " Mod-14 (anyTC 0) | "
    	    << " Mod-15 (anyTC 0) | "
	    << " Mod-p (anyTC 0) | "
    	    << " MB-E0 (anyTC 0) | "
	    << " MB-E1 (anyTC 0) | "
    
	    << " Mod-19 (S1MM) | "
    	    << " Mod-20 (S1MM) | "
    	    << " Mod-18 (S1MM) | "
	    << " Mod-16 (S1MM) | "
    	    << " Mod-17 (S1MM) | "
    	    << " Mod-14 (S1MM) | "
    	    << " Mod-15 (S1MM) | "
	    << " Mod-p (S1MM) | "
    	    << " MB-E0 (S1MM) | "
	    << " MB-E1 (S1MM) | "

	    << " Mod-19 (allbxdup) | "
    	    << " Mod-20 (allbxdup) | "
    	    << " Mod-18 (allbxdup) | "
	    << " Mod-16 (allbxdup) | "
    	    << " Mod-17 (allbxdup) | "
    	    << " Mod-14 (allbxdup) | "
    	    << " Mod-15 (allbxdup) | "
	    << " Mod-p (allbxdup) | "
    	    << " MB-E0 (allbxdup) | "
	    << " MB-E1 (allbxdup) | "
    
	    << " Mod-19 (alleldup) | "
    	    << " Mod-20 (alleldup) | "
    	    << " Mod-18 (alleldup) | "
	    << " Mod-16 (alleldup) | "
    	    << " Mod-17 (alleldup) | "
    	    << " Mod-14 (alleldup) | "
    	    << " Mod-15 (alleldup) | "
	    << " Mod-p (alleldup) | "
    	    << " MB-E0 (alleldup) | "
	    << " MB-E1 (alleldup ) | "

	    << " Mod-19 (anyeldup) | "
    	    << " Mod-20 (anyeldup) | "
    	    << " Mod-18 (anyeldup) | "
	    << " Mod-16 (anyeldup) | "
    	    << " Mod-17 (anyeldup) | "
    	    << " Mod-14 (anyeldup) | "
    	    << " Mod-15 (anyeldup) | "
	    << " Mod-p (anyeldup) | "
    	    << " MB-E0 (anyeldup) | "
	    << " MB-E1 (anyeldup ) | "
    
    	    << " Mod-19 (anyel 0) | "
    	    << " Mod-20 (anyel 0) | "
    	    << " Mod-18 (anyel 0) | "
	    << " Mod-16 (anyel 0) | "
    	    << " Mod-17 (anyel 0) | "
    	    << " Mod-14 (anyel 0) | "
    	    << " Mod-15 (anyel 0) | "
	    << " Mod-p (anyel 0) | "
    	    << " MB-E0 (anyel 0) | "
	    << " MB-E1 (anyel 0) | "

    	    << " Mod-19 (good) | "
    	    << " Mod-20 (good) | "
    	    << " Mod-18 (good) | "
	    << " Mod-16 (good) | "
    	    << " Mod-17 (good) | "
    	    << " Mod-14 (good) | "
    	    << " Mod-15 (good) | "
	    << " Mod-p (good) | "
    	    << " MB-E0 (good) | "
	    << " MB-E1 (good) | "

	    << std::endl;
  
  std::cerr << relayNumber << "| "
	    << runNumber << "| "
	    << (nofEvents-1) << "| "
	    << processed_events << "| "
	    <<std::fixed
    	    << std::setprecision(1)
    	    << std::setw(4)
	    << NofAllElZeroEvents[0][0].size() << " (" << 100.0*NofAllElZeroEvents[0][0].size()/processed_events << " %)" << "| "
    	    << NofAllElZeroEvents[0][1].size() << " (" << 100.0*NofAllElZeroEvents[0][1].size()/processed_events << " %)" << "| "
	    << NofAllElZeroEvents[0][2].size() << " (" << 100.0*NofAllElZeroEvents[0][2].size()/processed_events << " %)" << "| "
	    << NofAllElZeroEvents[1][0].size() << " (" << 100.0*NofAllElZeroEvents[1][0].size()/processed_events << " %)" << "| "
    	    << NofAllElZeroEvents[1][1].size() << " (" << 100.0*NofAllElZeroEvents[1][1].size()/processed_events << " %)" << "| "
	    << NofAllElZeroEvents[1][2].size() << " (" << 100.0*NofAllElZeroEvents[1][2].size()/processed_events << " %)" << "| "
	    << NofAllElZeroEvents[2][0].size() << " (" << 100.0*NofAllElZeroEvents[2][0].size()/processed_events << " %)" << "| "
    	    << NofAllElZeroEvents[2][1].size() << " (" << 100.0*NofAllElZeroEvents[2][1].size()/processed_events << " %)" << "| "
	    << NofAllElZeroEvents[3][0].size() << " (" << 100.0*NofAllElZeroEvents[3][0].size()/processed_events << " %)" << "| "
    	    << NofAllElZeroEvents[3][1].size() << " (" << 100.0*NofAllElZeroEvents[3][1].size()/processed_events << " %)" << "| "

	    << NofFixedPatEvents[0][0].size() << " (" << 100.0*NofFixedPatEvents[0][0].size()/processed_events << " %)" << "| "
    	    << NofFixedPatEvents[0][1].size() << " (" << 100.0*NofFixedPatEvents[0][1].size()/processed_events << " %)" << "| "
	    << NofFixedPatEvents[0][2].size() << " (" << 100.0*NofFixedPatEvents[0][2].size()/processed_events << " %)" << "| "
	    << NofFixedPatEvents[1][0].size() << " (" << 100.0*NofFixedPatEvents[1][0].size()/processed_events << " %)" << "| "
    	    << NofFixedPatEvents[1][1].size() << " (" << 100.0*NofFixedPatEvents[1][1].size()/processed_events << " %)" << "| "
	    << NofFixedPatEvents[1][2].size() << " (" << 100.0*NofFixedPatEvents[1][2].size()/processed_events << " %)" << "| "
	    << NofFixedPatEvents[2][0].size() << " (" << 100.0*NofFixedPatEvents[2][0].size()/processed_events << " %)" << "| "
    	    << NofFixedPatEvents[2][1].size() << " (" << 100.0*NofFixedPatEvents[2][1].size()/processed_events << " %)" << "| "
	    << NofFixedPatEvents[3][0].size() << " (" << 100.0*NofFixedPatEvents[3][0].size()/processed_events << " %)" << "| "
    	    << NofFixedPatEvents[3][1].size() << " (" << 100.0*NofFixedPatEvents[3][1].size()/processed_events << " %)" << "| "

	    << NofAllZeroEvents[0][0].size() << " (" << 100.0*NofAllZeroEvents[0][0].size()/processed_events << " %)" << "| "
    	    << NofAllZeroEvents[0][1].size() << " (" << 100.0*NofAllZeroEvents[0][1].size()/processed_events << " %)" << "| "
	    << NofAllZeroEvents[0][2].size() << " (" << 100.0*NofAllZeroEvents[0][2].size()/processed_events << " %)" << "| "
	    << NofAllZeroEvents[1][0].size() << " (" << 100.0*NofAllZeroEvents[1][0].size()/processed_events << " %)" << "| "
    	    << NofAllZeroEvents[1][1].size() << " (" << 100.0*NofAllZeroEvents[1][1].size()/processed_events << " %)" << "| "
	    << NofAllZeroEvents[1][2].size() << " (" << 100.0*NofAllZeroEvents[1][2].size()/processed_events << " %)" << "| "
	    << NofAllZeroEvents[2][0].size() << " (" << 100.0*NofAllZeroEvents[2][0].size()/processed_events << " %)" << "| "
    	    << NofAllZeroEvents[2][1].size() << " (" << 100.0*NofAllZeroEvents[2][1].size()/processed_events << " %)" << "| "
	    << NofAllZeroEvents[3][0].size() << " (" << 100.0*NofAllZeroEvents[3][0].size()/processed_events << " %)" << "| "
    	    << NofAllZeroEvents[3][1].size() << " (" << 100.0*NofAllZeroEvents[3][1].size()/processed_events << " %)" << "| "

	    << NofAnyZeroEvents[0][0].size() << " (" << 100.0*NofAnyZeroEvents[0][0].size()/processed_events << " %)" << "| "
    	    << NofAnyZeroEvents[0][1].size() << " (" << 100.0*NofAnyZeroEvents[0][1].size()/processed_events << " %)" << "| "
	    << NofAnyZeroEvents[0][2].size() << " (" << 100.0*NofAnyZeroEvents[0][2].size()/processed_events << " %)" << "| "
	    << NofAnyZeroEvents[1][0].size() << " (" << 100.0*NofAnyZeroEvents[1][0].size()/processed_events << " %)" << "| "
    	    << NofAnyZeroEvents[1][1].size() << " (" << 100.0*NofAnyZeroEvents[1][1].size()/processed_events << " %)" << "| "
	    << NofAnyZeroEvents[1][2].size() << " (" << 100.0*NofAnyZeroEvents[1][2].size()/processed_events << " %)" << "| "
	    << NofAnyZeroEvents[2][0].size() << " (" << 100.0*NofAnyZeroEvents[2][0].size()/processed_events << " %)" << "| "
    	    << NofAnyZeroEvents[2][1].size() << " (" << 100.0*NofAnyZeroEvents[2][1].size()/processed_events << " %)" << "| "
	    << NofAnyZeroEvents[3][0].size() << " (" << 100.0*NofAnyZeroEvents[3][0].size()/processed_events << " %)" << "| "
    	    << NofAnyZeroEvents[3][1].size() << " (" << 100.0*NofAnyZeroEvents[3][1].size()/processed_events << " %)" << "| "

	    << NofS1EmulDataMM[0][0].size() << " (" << 100.0*NofS1EmulDataMM[0][0].size()/processed_events << " %)" << "| "
    	    << NofS1EmulDataMM[0][1].size() << " (" << 100.0*NofS1EmulDataMM[0][1].size()/processed_events << " %)" << "| "
	    << NofS1EmulDataMM[0][2].size() << " (" << 100.0*NofS1EmulDataMM[0][2].size()/processed_events << " %)" << "| "
	    << NofS1EmulDataMM[1][0].size() << " (" << 100.0*NofS1EmulDataMM[1][0].size()/processed_events << " %)" << "| "
    	    << NofS1EmulDataMM[1][1].size() << " (" << 100.0*NofS1EmulDataMM[1][1].size()/processed_events << " %)" << "| "
	    << NofS1EmulDataMM[1][2].size() << " (" << 100.0*NofS1EmulDataMM[1][2].size()/processed_events << " %)" << "| "
	    << NofS1EmulDataMM[2][0].size() << " (" << 100.0*NofS1EmulDataMM[2][0].size()/processed_events << " %)" << "| "
    	    << NofS1EmulDataMM[2][1].size() << " (" << 100.0*NofS1EmulDataMM[2][1].size()/processed_events << " %)" << "| "
	    << NofS1EmulDataMM[3][0].size() << " (" << 100.0*NofS1EmulDataMM[3][0].size()/processed_events << " %)" << "| "
    	    << NofS1EmulDataMM[3][1].size() << " (" << 100.0*NofS1EmulDataMM[3][1].size()/processed_events << " %)" << "| "

	    << NofAllElDupEvents[0][0].size() << " (" << 100.0*NofAllElDupEvents[0][0].size()/processed_events << " %)" << "| "
    	    << NofAllElDupEvents[0][1].size() << " (" << 100.0*NofAllElDupEvents[0][1].size()/processed_events << " %)" << "| "
	    << NofAllElDupEvents[0][2].size() << " (" << 100.0*NofAllElDupEvents[0][2].size()/processed_events << " %)" << "| "
	    << NofAllElDupEvents[1][0].size() << " (" << 100.0*NofAllElDupEvents[1][0].size()/processed_events << " %)" << "| "
    	    << NofAllElDupEvents[1][1].size() << " (" << 100.0*NofAllElDupEvents[1][1].size()/processed_events << " %)" << "| "
	    << NofAllElDupEvents[1][2].size() << " (" << 100.0*NofAllElDupEvents[1][2].size()/processed_events << " %)" << "| "
	    << NofAllElDupEvents[2][0].size() << " (" << 100.0*NofAllElDupEvents[2][0].size()/processed_events << " %)" << "| "
    	    << NofAllElDupEvents[2][1].size() << " (" << 100.0*NofAllElDupEvents[2][1].size()/processed_events << " %)" << "| "
	    << NofAllElDupEvents[3][0].size() << " (" << 100.0*NofAllElDupEvents[3][0].size()/processed_events << " %)" << "| "
    	    << NofAllElDupEvents[3][1].size() << " (" << 100.0*NofAllElDupEvents[3][1].size()/processed_events << " %)" << "| "

	    << NofAllSameElinksEvents[0][0].size() << " (" << 100.0*NofAllSameElinksEvents[0][0].size()/processed_events << " %)" << "| "
    	    << NofAllSameElinksEvents[0][1].size() << " (" << 100.0*NofAllSameElinksEvents[0][1].size()/processed_events << " %)" << "| "
	    << NofAllSameElinksEvents[0][2].size() << " (" << 100.0*NofAllSameElinksEvents[0][2].size()/processed_events << " %)" << "| "
	    << NofAllSameElinksEvents[1][0].size() << " (" << 100.0*NofAllSameElinksEvents[1][0].size()/processed_events << " %)" << "| "
    	    << NofAllSameElinksEvents[1][1].size() << " (" << 100.0*NofAllSameElinksEvents[1][1].size()/processed_events << " %)" << "| "
	    << NofAllSameElinksEvents[1][2].size() << " (" << 100.0*NofAllSameElinksEvents[1][2].size()/processed_events << " %)" << "| "
	    << NofAllSameElinksEvents[2][0].size() << " (" << 100.0*NofAllSameElinksEvents[2][0].size()/processed_events << " %)" << "| "
    	    << NofAllSameElinksEvents[2][1].size() << " (" << 100.0*NofAllSameElinksEvents[2][1].size()/processed_events << " %)" << "| "
	    << NofAllSameElinksEvents[3][0].size() << " (" << 100.0*NofAllSameElinksEvents[3][0].size()/processed_events << " %)" << "| "
    	    << NofAllSameElinksEvents[3][1].size() << " (" << 100.0*NofAllSameElinksEvents[3][1].size()/processed_events << " %)" << "| "

	    << NofDuplicateElinksEvents[0][0].size() << " (" << 100.0*NofDuplicateElinksEvents[0][0].size()/processed_events << " %)" << "| "
    	    << NofDuplicateElinksEvents[0][1].size() << " (" << 100.0*NofDuplicateElinksEvents[0][1].size()/processed_events << " %)" << "| "
	    << NofDuplicateElinksEvents[0][2].size() << " (" << 100.0*NofDuplicateElinksEvents[0][2].size()/processed_events << " %)" << "| "
	    << NofDuplicateElinksEvents[1][0].size() << " (" << 100.0*NofDuplicateElinksEvents[1][0].size()/processed_events << " %)" << "| "
    	    << NofDuplicateElinksEvents[1][1].size() << " (" << 100.0*NofDuplicateElinksEvents[1][1].size()/processed_events << " %)" << "| "
	    << NofDuplicateElinksEvents[1][2].size() << " (" << 100.0*NofDuplicateElinksEvents[1][2].size()/processed_events << " %)" << "| "
	    << NofDuplicateElinksEvents[2][0].size() << " (" << 100.0*NofDuplicateElinksEvents[2][0].size()/processed_events << " %)" << "| "
    	    << NofDuplicateElinksEvents[2][1].size() << " (" << 100.0*NofDuplicateElinksEvents[2][1].size()/processed_events << " %)" << "| "
	    << NofDuplicateElinksEvents[3][0].size() << " (" << 100.0*NofDuplicateElinksEvents[3][0].size()/processed_events << " %)" << "| "
    	    << NofDuplicateElinksEvents[3][1].size() << " (" << 100.0*NofDuplicateElinksEvents[3][1].size()/processed_events << " %)" << "| "
    
    	    << NofAnyElZeroEvents[0][0].size() << " (" << 100.0*NofAnyElZeroEvents[0][0].size()/processed_events << " %)" << "| "
    	    << NofAnyElZeroEvents[0][1].size() << " (" << 100.0*NofAnyElZeroEvents[0][1].size()/processed_events << " %)" << "| "
	    << NofAnyElZeroEvents[0][2].size() << " (" << 100.0*NofAnyElZeroEvents[0][2].size()/processed_events << " %)" << "| "
	    << NofAnyElZeroEvents[1][0].size() << " (" << 100.0*NofAnyElZeroEvents[1][0].size()/processed_events << " %)" << "| "
    	    << NofAnyElZeroEvents[1][1].size() << " (" << 100.0*NofAnyElZeroEvents[1][1].size()/processed_events << " %)" << "| "
	    << NofAnyElZeroEvents[1][2].size() << " (" << 100.0*NofAnyElZeroEvents[1][2].size()/processed_events << " %)" << "| "
	    << NofAnyElZeroEvents[2][0].size() << " (" << 100.0*NofAnyElZeroEvents[2][0].size()/processed_events << " %)" << "| "
    	    << NofAnyElZeroEvents[2][1].size() << " (" << 100.0*NofAnyElZeroEvents[2][1].size()/processed_events << " %)" << "| "
	    << NofAnyElZeroEvents[3][0].size() << " (" << 100.0*NofAnyElZeroEvents[3][0].size()/processed_events << " %)" << "| "
    	    << NofAnyElZeroEvents[3][1].size() << " (" << 100.0*NofAnyElZeroEvents[3][1].size()/processed_events << " %)" << "| "
    
	    << NofGoodEvents[0][0].size() << " (" << 100.0*NofGoodEvents[0][0].size()/processed_events << " %)" << "| "
    	    << NofGoodEvents[0][1].size() << " (" << 100.0*NofGoodEvents[0][1].size()/processed_events << " %)" << "| "
	    << NofGoodEvents[0][2].size() << " (" << 100.0*NofGoodEvents[0][2].size()/processed_events << " %)" << "| "
	    << NofGoodEvents[1][0].size() << " (" << 100.0*NofGoodEvents[1][0].size()/processed_events << " %)" << "| "
    	    << NofGoodEvents[1][1].size() << " (" << 100.0*NofGoodEvents[1][1].size()/processed_events << " %)" << "| "
	    << NofGoodEvents[1][2].size() << " (" << 100.0*NofGoodEvents[1][2].size()/processed_events << " %)" << "| "
	    << NofGoodEvents[2][0].size() << " (" << 100.0*NofGoodEvents[2][0].size()/processed_events << " %)" << "| "
    	    << NofGoodEvents[2][1].size() << " (" << 100.0*NofGoodEvents[2][1].size()/processed_events << " %)" << "| "
	    << NofGoodEvents[3][0].size() << " (" << 100.0*NofGoodEvents[3][0].size()/processed_events << " %)" << "| "
    	    << NofGoodEvents[3][1].size() << " (" << 100.0*NofGoodEvents[3][1].size()/processed_events << " %)" << "| "
    
   	    << std::endl;

  return true;
}

uint64_t getTotalNofEvents(uint32_t relayNumber, uint32_t runNumber){

  uint64_t nEvents(0);
  //Create the file reader
  Hgcal10gLinkReceiver::FileReader fReader;
  
  //Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);
  
  //Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);
  fReader.setDirectory(std::string("dat/Relay")+std::to_string(relayNumber));
  //fReader.openRun(runNumber,linkNumber);
  //We open linkNumber=0 which contains trigger data
  fReader.openRun(runNumber,0); 
  
  while(fReader.read(r)) {
    //Check the state of the record and print the record accordingly
    if( r->state()==Hgcal10gLinkReceiver::FsmState::Starting){
      if(!(rStart->valid())){
  	std::cerr << " FsmState::Starting validadity fails : rStart->valid() " << rStart->valid() << std::endl;
  	rStart->print();
  	std::cout << std::endl;
	continue;
      }
    }
    
    else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping){
      if(!(rStop->valid())){
  	std::cerr << " FsmState::Stopping validadity fails : rStop->valid() " << rStop->valid() << std::endl;
  	rStop->print();
  	std::cout << std::endl;
	continue;
      }
    }
    //Else we have an event record 
    else{
      nEvents++;      
    }//loop event
  }
  
  std::cout << "========== Total Events : " << nEvents << std::endl;
  
  delete r;

  return nEvents;
}

