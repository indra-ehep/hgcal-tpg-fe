/**********************************************************************
 Created on : 14/07/2024
 Purpose    : Read the econt data for test-beam of July 2024
 Author     : Indranil Das, Visiting Fellow
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
#include "common/inc/FileReader.h"

#include "yaml-cpp/yaml.h"
#include <deque>

#include <string>
#include <fstream>
#include <cassert>

#include "TPGFEDataformat.hh"
#include "TPGFEConfiguration.hh"
#include "TPGFEReader2024.hh"


int main(int argc, char** argv)
{
  //===============================================================================================================================
  // ./compile.sh
  // ./read_econt_Jul24.exe $Relay $rname $link_number
  //===============================================================================================================================  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return false;
  }
  if(argc < 4){
    std::cerr << argv[1] << ": no link number (1 or 2) is specified " << std::endl;
    return false;
  }
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Assign relay,run and link numbers from input
  //===============================================================================================================================
  uint32_t relayNumber(0);
  uint32_t runNumber(0);
  uint32_t linkNumber(0);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  std::istringstream issLink(argv[3]);
  issLink >> linkNumber;
  if(linkNumber!=1 and linkNumber!=2){
    std::cerr << "Link number "<< argv[3] <<"is out of bound (use: 1 or 2)" << std::endl;
    return false;
  }
  uint32_t trig_linkNumber = TMath::FloorNint((linkNumber-1)/2);
  int isMSB = 1;
  if(linkNumber==1) isMSB = 0;
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
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Read ECON-T setting
  //===============================================================================================================================  
  uint32_t zside = 0, sector = 0, link = 0, det = 0;
  uint32_t econt = 0, selTC4 = 1, module = 0;
  TPGFEConfiguration::TPGFEIdPacking pck;

  for(int ilink=0;ilink<2;ilink++){
    for(int iecond=0;iecond<3;iecond++){
      uint32_t idx = pck.packModId(zside, sector, ilink, det, iecond, selTC4, module); //we assume same ECONT and ECOND number for a given module
      
      cfgs.setEconDFile(Form("cfgmap/init_econd.yaml",relayNumber));
      cfgs.readEconDConfigYaml(idx);
      
      cfgs.setEconTFile(Form("cfgmap/init_econt.yaml",relayNumber));
      cfgs.readEconTConfigYaml(idx);
    }//econd loop
  }//lpGBT loop
  //===============================================================================================================================
  
  std::map<uint32_t,TPGFEConfiguration::ConfigEconD>& econDPar =  cfgs.getEconDPara();
  for(const auto& it : econDPar){
    std::cout << "it.first: "<< it.first << std::endl;
    econDPar.at(it.first).setPassThrough(true);
    econDPar.at(it.first).setNeRx(6);
  }
  
  //===============================================================================================================================
  //Set ECON-T parameters manually (as it was for September, 2024 beam-test)
  //===============================================================================================================================
  
  link = 0; econt = 0;
  uint32_t lp0_bc = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  econt = 1;
  uint32_t lp0_stc4a = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  econt = 2;
  uint32_t lp0_stc16orctc4a = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  
  link = 1; econt = 0;
  uint32_t lp1_bc = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  econt = 1;
  uint32_t lp1_stc4a = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  econt = 2;
  uint32_t lp1_stc16orctc4a = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar =  cfgs.getEconTPara();
  for(const auto& it : econTPar){
    econTPar[it.first].setDensity(1);
    econTPar[it.first].setDropLSB(1);
    if(it.first==lp0_bc or it.first==lp1_bc){
      //following for BC6
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(3);
    }else if(it.first==lp0_stc4a or it.first==lp1_stc4a){
      //following for STC4A
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setSTCType(3);
    }else{
      if(relayNumber<=1722876649){         //both laters with CTC4A
	econTPar[it.first].setSelect(1);
	econTPar[it.first].setSTCType(2);
      }else if(relayNumber==1722881092){   //one later with CTC4A and another with STC16
	if(it.first==lp0_stc16orctc4a){
	  //link0 is set for CTC4A
	  econTPar[it.first].setSelect(1);
	  econTPar[it.first].setSTCType(2);
	}
	if(it.first==lp1_stc16orctc4a){
	  //link0 is set for STC16
	  econTPar[it.first].setSelect(1);
	  econTPar[it.first].setSTCType(1);
	}
      }else{                               //both laters with STC16
	econTPar[it.first].setSelect(1);
	econTPar[it.first].setSTCType(1);
      }
    }
    
    econTPar[it.first].setCalibration(0x800);
  }//econt loop
  //===============================================================================================================================

  // ===============================================================================================================================
  // Set and Initialize the ECONT reader
  // ===============================================================================================================================
  TPGFEReader::ECONTReader econTReader(cfgs);
  //output array
  //econTReader.checkEvent(1);
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::Trig24Data>>> econtarray; //event,moduleId (from link)
  //void ECONTReader::getEvents(uint64_t& minEventTrig, uint64_t& maxEventTrig, std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::Trig24Data>>>& econtarray)
  // ===============================================================================================================================
  
  //===============================================================================================================================
  //Set and Initialize the ECOND reader
  //===============================================================================================================================
  TPGFEReader::ECONDReader econDReader(cfgs);
  econDReader.setTotUp(0);
  //econDReader.checkEvent(1);
  //econDReader.showFirstEvents(10);
  
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>> hrocarray; //event,rocId
  std::vector<uint64_t> eventList;
  uint64_t minEventDAQ = 0, maxEventDAQ = 10;
  
  hrocarray.clear();
  eventList.clear();
  econtarray.clear();
  
  econTReader.init(relayNumber,runNumber,0);
  std::cout<<"TRIG Before Link "<<trig_linkNumber<<", size : " << econtarray.size() <<std::endl;
  econTReader.getEvents(minEventDAQ, maxEventDAQ, econtarray);
  std::cout<<"TRIG After Link "<<trig_linkNumber<<", size : " << econtarray.size() <<std::endl;
  econTReader.terminate();

  econDReader.init(relayNumber,runNumber,linkNumber);
  //econDReader.setModulePath(zside, sector, link, det, econt, selTC4, module);
  econDReader.getEvents(minEventDAQ, maxEventDAQ, hrocarray, eventList); //Input <---> output
  econDReader.terminate();
  
  
  for(const auto& ievent : eventList){
    std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> datalist = hrocarray[ievent] ;
    std::cout << "ievent: "<< ievent<< ", datalist size: "<< datalist.size() << std::endl;
    for(const auto& hrocit : datalist){
      TPGFEDataformat::HalfHgcrocData hrocdata = hrocit.second ;
      std::cout << "\t data for half-roc_id: "<< hrocit.first <<  std::endl;
      //hrocdata.print();
    }
    std::vector<std::pair<uint32_t,TPGFEDataformat::Trig24Data>> econtdata =  econtarray[ievent]; 
    for(const auto& econtit : econtdata){
      std::cout << "\t data for econt_id: "<< econtit.first <<  std::endl;
      TPGFEDataformat::Trig24Data trdata = econtit.second ;
      //trdata.print();
    }
  }
  // //===============================================================================================================================
  // //Read one event
  // //===============================================================================================================================
  // uint64_t event = 10;
  // econTReader.getEvents(event, econtarray);
  // //===============================================================================================================================
  
  
  // //===============================================================================================================================
  // //Print the read value
  // //===============================================================================================================================
  // const std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>& vecont = econtarray.at(event);
  // for(const auto& modpair : vecont){
  //   const uint32_t& modnum = modpair.first;
  //   const std::vector<TPGFEDataformat::TcRawData>& econtlist = modpair.second;
  //   std::cout <<"Event: "<< event << ", Module: " << modnum << ", size: " << econtlist.size() << std::endl;
  //   for(size_t itc=0 ; itc < econtlist.size() ; itc++){
  //     const TPGFEDataformat::TcRawData& tcedata = econtlist.at(itc);
  //     tcedata.print();
  //   }//loop to print the TC/STC results
  // }//modules
  // //===============================================================================================================================

  // //delete the file readers
  // econTReader.terminate();
  
  return true;
}
