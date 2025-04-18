/**********************************************************************
 Created on : 18/10/2024
 Purpose    : Study of TPG data for 2 silicon trains, 1 motherboard and external trigger
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
#include "TPGFEReaderSep2024.hh"
#include "Stage1IO.hh"

int main(int argc, char** argv)
{
  
  std::cout << "========= tpgdata_2T_fe==========" << std::endl;
  
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
  
  if(relayNumber>=1727007636){
    std::cerr << "Relaynumber should corresponds to those with two silicon layers configuration" << std::endl;
    return false;
  }
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  if(argc = 4){
    std::istringstream issnEvents(argv[3]);
    issnEvents >> nofEvents;
  }
  totEvents = getTotalNofEvents(relayNumber, runNumber);
  if(nofEvents>totEvents){
    nofEvents = totEvents ;
    std::cout << "Setting number of events to process to " << nofEvents << std::endl;
  }
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
  uint32_t noflpGBTs = 4;
  TPGFEConfiguration::TPGFEIdPacking pck;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=2)?3:2;
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
  link = 1; econt = 0; uint32_t lp1_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 1; uint32_t lp1_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 2; uint32_t lp1_stc4a2 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 2; econt = 0; uint32_t lp2_stc160 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 2; econt = 1; uint32_t lp2_stc161 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 2; econt = 2; uint32_t lp2_stc162 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
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
    }else if (it.first==lp1_stc4a0){    //// starting 2nd train
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(3);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp1_stc4a1){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp1_stc4a2){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp2_stc160){    //// starting 3rd train
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(3);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp2_stc161){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp2_stc162){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
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
  econTReader.setNofCAFESep(11);
  // ===============================================================================================================================
  
  //===============================================================================================================================
  //Process certain refernce eventlist
  //===============================================================================================================================
  /*TcTp1 events */ uint64_t refEvt[2] = {19, 137};
  std::vector<uint64_t> refEvents;
  for(int ievent=0;ievent<2;ievent++) refEvents.push_back(refEvt[ievent]);
  //when the following line is "commented" only the events filled in "refEvents" will be processed instead of nofevents set in the command line
  //refEvents.resize(0);
  //===============================================================================================================================

  uint64_t minEventTPG, maxEventTPG;  
  long double nloopEvent =  100000;
  int nloop = TMath::CeilNint(nofEvents/nloopEvent) ;
  if(refEvents.size()>0) nloop = 1;
  std::cout<<"nloop: "<<nloop<<std::endl;

  for(int ieloop=0;ieloop<nloop;ieloop++){
    
    minEventTPG = ieloop*nloopEvent ;
    maxEventTPG = (ieloop+1)*nloopEvent;
    maxEventTPG = (maxEventTPG>uint64_t(nofEvents)) ? uint64_t(nofEvents) : maxEventTPG ;
    printf("iloop : %d, minEventTPG = %lu, maxEventTPG = %lu\n",ieloop,minEventTPG, maxEventTPG);
    std::cerr<<"iloop : "<< ieloop <<", minEventTPG = "<< minEventTPG <<", maxEventTPG = " << maxEventTPG << std::endl;

    econtarray.clear();
    
    econTReader.init(relayNumber,runNumber,0);
    std::cout<<"TRIG Before: elink and unpacker size : " << econtarray.size() <<std::endl;
    econTReader.getEvents(refEvents, minEventTPG, maxEventTPG, econtarray);
    std::cout<<"TRIG After:  elink and unpacker size : " << econtarray.size() <<std::endl;
    econTReader.terminate();
    
    for(const auto& it :  econtarray){ //this is essencially event loop
      uint64_t event = it.first ;
      std::cout << "Processing event: " << event << std::endl;
      for(int ilink=0;ilink<noflpGBTs;ilink++){
	int maxecons = (ilink<=2)?3:2;
	for(int iecont=0;iecont<maxecons;iecont++){
	  uint32_t moduleId = pck.packModId(zside, sector, ilink, det, iecont, selTC4, module); 
	  std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>> econtdata =  econtarray[event];	  
	  TPGBEDataformat::UnpackerOutputStreamPair upemul,updata;
	  TPGFEDataformat::TcRawDataPacket vTCel;	
	  TPGBEDataformat::Trig24Data trdata;
	  
	  for(const auto& econtit : econtdata){
	    if(econtit.first!=moduleId) continue;
	    trdata = econtit.second ;
	    std::cout << "Dataloop:: event: " << event << ", moduleId : " << econtit.first << std::endl;
	    //for(int ibx=0;ibx<7;ibx++){
	    for(int ibx=3;ibx<4;ibx++){
	      const uint32_t *el = trdata.getElinks(ibx); 
	      uint32_t bx_2 = (el[0]>>28) & 0xF;
	      if(ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, econTPar[moduleId].getNofTCs(), el, vTCel);
	      if(ilink==2) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, econTPar[moduleId].getNofSTCs(), el, vTCel);
	      if(ilink==1 or ilink==3) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, econTPar[moduleId].getNofSTCs(), el, vTCel);	      
	      TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(bx_2, vTCel, upemul);
	      std::cout << "========== ibx : " << ibx << " ==========" << std::endl;
	      std::cout << "========== TC as read from elink for ibx : " << ibx << " ==========" << std::endl;
	      vTCel.print();
	      std::cout << "========== Emulated Stage1 unpacker output stream from TC for ibx : " << ibx << " ==========" << std::endl;
	      upemul.print();
	      std::cout << "========== TPG data: Stage1 unpacker output stream from firmware output for ibx : " << ibx << " ==========" << std::endl;
	      trdata.getUnpkStream(ibx,updata);
	      updata.print();
	      std::cout << "========== end of TPG data for ibx : " << ibx << " ==========" << std::endl;
	    }//bx loop
	    //if(eventCondn) trdata.print();
	  }//loop over econt data for a given run

	}//econt loop
      }//link loop
    }//this is essencially event loop
    
  }//ieloop
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

