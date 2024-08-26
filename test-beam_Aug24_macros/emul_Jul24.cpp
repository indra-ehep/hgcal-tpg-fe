/**********************************************************************
 Created on : 13/08/2024
 Purpose    : Emulation for test-beam of July-August 2024
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
#include "TPGBEDataformat.hh"
#include "TPGFEConfiguration.hh"
#include "TPGFEModuleEmulation.hh"
#include "TPGFEReader2024.hh"
#include "Stage1IO.hh"

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
  //Read ECON-T setting
  //===============================================================================================================================  
  uint32_t zside = 0, sector = 0, link = 0, det = 0;
  uint32_t econt = 0, selTC4 = 1, module = 0;
  TPGFEConfiguration::TPGFEIdPacking pck;
  
  std::string cfgrocname[2][3][3] = {
    {
      {"320MLF3WXIH0012_roc0_e2.yaml", "320MLF3WXIH0012_roc1_e2.yaml", "320MLF3WXIH0012_roc2_e2.yaml"},
      {"320MLF3WXIH0006_roc0_e1.yaml", "320MLF3WXIH0006_roc1_e1.yaml", "320MLF3WXIH0006_roc2_e1.yaml"},
      {"320MLF3WXIH0011_roc0_e0.yaml", "320MLF3WXIH0011_roc1_e0.yaml", "320MLF3WXIH0011_roc2_e0.yaml"}
    },
    {
      {"320MLF3WXIH0007_roc0_e2.yaml", "320MLF3WXIH0007_roc1_e2.yaml", "320MLF3WXIH0007_roc2_e2.yaml"},
      {"320MLF3WXIH0005_roc0_e1.yaml", "320MLF3WXIH0005_roc1_e1.yaml", "320MLF3WXIH0005_roc2_e1.yaml"},
      {"320MLF3WXIH0010_roc0_e0.yaml", "320MLF3WXIH0010_roc1_e0.yaml", "320MLF3WXIH0010_roc2_e0.yaml"}
    }
  };
  
  for(int ilink=0;ilink<2;ilink++){
    for(int iecond=0;iecond<3;iecond++){
      uint32_t idx = pck.packModId(zside, sector, ilink, det, iecond, selTC4, module); //we assume same ECONT and ECOND number for a given module
      
      cfgs.setModulePath(zside, sector, ilink, det, iecond, selTC4, module);
      
      cfgs.setEconDFile("cfgmap/slow_control_configuration/init_econd.yaml");
      cfgs.readEconDConfigYaml();
      
      cfgs.setEconTFile("cfgmap/slow_control_configuration/init_econt.yaml");
      cfgs.readEconTConfigYaml();
      
      if(relayNumber==1722870998 and runNumber==1722871004){
	if(ilink==1 and (iecond==1 or iecond==2)){
	  cfgs.setEconTFile("cfgmap/init_econt_e2_cal_setup2.yaml");
	  cfgs.readEconTConfigYaml();
	}
	if(ilink==1 and (iecond==0)){
	  cfgs.setEconTFile("cfgmap/init_econt_e2_mux_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	}
      }

      if(relayNumber==1722871297 and runNumber==1722871302){
	if(ilink==1 and (iecond==1 or iecond==2)){
	  cfgs.setEconTFile("cfgmap/init_econt_e1_cal_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	}
	if(ilink==1 and (iecond==0)){
	  cfgs.setEconTFile("cfgmap/init_econt_e2_mux_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	}
      }

      if(relayNumber==1722871625 and runNumber==1722871630){
	if(ilink==1 and (iecond==1 or iecond==2)){
	  cfgs.setEconTFile("cfgmap/init_econt_e1_cal_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	  cfgs.setEconTFile("cfgmap/init_econt_e1_mux_setup2.yaml");
	  cfgs.readEconTConfigYaml();
	}
	if(ilink==1 and (iecond==0)){
	  cfgs.setEconTFile("cfgmap/init_econt_e2_mux_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	}
      }

      if(relayNumber==1722871974 and runNumber==1722871979){
	if(ilink==1 and (iecond==1 or iecond==2)){
	  cfgs.setEconTFile("cfgmap/init_econt_e1_cal_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	  cfgs.setEconTFile("cfgmap/init_econt_e1_mux_setup2.yaml");
	  cfgs.readEconTConfigYaml();
	}
	if(ilink==1 and (iecond==0)){
	  cfgs.setEconTFile("cfgmap/init_econt_e2_cal_setup2.yaml");
	  cfgs.readEconTConfigYaml();
	  cfgs.setEconTFile("cfgmap/init_econt_e2_mux_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	}
      }

      
      for(uint32_t iroc=0;iroc<3;iroc++){
	std::cout<<"idx: "<<idx<<", ilink: " << ilink << ", iecond: "<<iecond<<", iroc: "<<iroc << ", fname : " << cfgrocname[ilink][iecond][iroc] << std::endl;
	uint32_t rocid_0 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 0);
	uint32_t rocid_1 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 1);
	//cfgs.setRocFile(Form("cfgmap/slow_control_configuration/configs_with_MasterTDC_and_TPGTRM/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	cfgs.setRocFile(Form("cfgmap/bt2024-fe-config-trimmed/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	cfgs.readRocConfigYaml(rocid_0, rocid_1);	
      }//roc loop
    }//econd loop
  }//lpGBT loop
  //cfgs.setPedThZero();
  link = 0; econt = 1;
  uint32_t testmodid = pck.packModId(zside, sector, link, det, econt, selTC4, module); //we assume same ECONT and ECOND number for a given module
  cfgs.printCfgPedTh(testmodid);
  //===============================================================================================================================
  
  std::map<uint32_t,TPGFEConfiguration::ConfigEconD>& econDPar =  cfgs.getEconDPara();
  for(const auto& it : econDPar){
    //std::cout << "it.first: "<< it.first << std::endl;
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
      econTPar[it.first].setNElinks(2);
    }else{
      if(relayNumber<=1722876649){         //both laters with CTC4A
	econTPar[it.first].setSelect(1);
	econTPar[it.first].setSTCType(2);
	econTPar[it.first].setNElinks(2);
      }else if(relayNumber==1722881092){   //one later with CTC4A and another with STC16
	if(it.first==lp0_stc16orctc4a){
	  //link0 is set for CTC4A
	  econTPar[it.first].setSelect(1);
	  econTPar[it.first].setSTCType(2);
	  econTPar[it.first].setNElinks(2);
	}
	if(it.first==lp1_stc16orctc4a){
	  //link0 is set for STC16
	  econTPar[it.first].setSelect(1);
	  econTPar[it.first].setSTCType(1);
	  econTPar[it.first].setNElinks(2);
	}
      }else{                               //both laters with STC16
	econTPar[it.first].setSelect(1);
	econTPar[it.first].setSTCType(1);
	econTPar[it.first].setNElinks(2);
      }
    }
    std::cout << "Modtype " << it.first << ", getOuttype: " << econTPar[it.first].getOutType() << ", typename: " << TPGFEDataformat::tctypeName[econTPar[it.first].getOutType()] << std::endl;
    econTPar[it.first].print();
    //for(uint32_t itc=0;itc<48;itc++) econTPar[it.first].setCalibration(itc,0x800);
  }//econt loop
  //===============================================================================================================================

  // ===============================================================================================================================
  // Set and Initialize the ECONT reader
  // ===============================================================================================================================
  TPGFEReader::ECONTReader econTReader(cfgs);
  //output array
  //econTReader.checkEvent(1);
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>>> econtarray; //event,moduleId (from link)
  //void ECONTReader::getEvents(uint64_t& minEventTrig, uint64_t& maxEventTrig, std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>>>& econtarray)
  // ===============================================================================================================================
  
  //===============================================================================================================================
  //Set and Initialize the ECOND reader
  //===============================================================================================================================
  TPGFEReader::ECONDReader econDReader(cfgs);
  econDReader.setTotUp(0);
  //econDReader.checkEvent(1);
  //econDReader.showFirstEvents(10);

  //===============================================================================================================================
  //Set and Initialize the Emulator
  //===============================================================================================================================
  TPGFEModuleEmulation::HGCROCTPGEmulation rocTPGEmul(cfgs);
  TPGFEModuleEmulation::ECONTEmulation econtEmul(cfgs);
  //===============================================================================================================================

  //===============================================================================================================================
  //Set refernce eventlist
  //===============================================================================================================================
  // uint64_t refEvt[12] = {1125, 3855, 4350, 14641,
  //                     21453, 24865, 27540, 31430,
  // 		      33248, 36473, 38454, 39722};
  //// ./emul_Jul24.exe 1722870998 1722871004 1 > temp.log  
  //uint64_t refEvt[3] = {12049, 16652, 17546}; //BC lpGBT:1
  //uint64_t refEvt[14] = {868, 6064, 6114, 6590, 8204, 8976, 11050, 12143, 19103, 21388, 21464, 22618, 23434, 23778}; //STC4A lpGBT:1
  //uint64_t refEvt[8] = {563, 11321, 12346, 12646, 17305, 19916, 22529, 22857}; //CTC4A lpGBT:1
  
  ////./emul_Jul24.exe 1722871974 1722871979 1 > temp1.log
  uint64_t refEvt[2] = {9198, 16570}; // BC lpGBT:1
  //uint64_t refEvt[24] = {886, 2006, 2236, 2757, 3349, 4520, 6407, 6945, 11617, 13469, 14599, 17199, 19673, 20470, 20827, 21444, 22509, 23358, 26502, 26577, 26627, 30012, 30853, 31140}; //BC lpGBT:0
  //uint64_t refEvt[73] = {143, 567, 728, 1895, 1994, 2062, 2099, 2273, 2481, 2532, 2598, 2938, 5501, 6559, 6931, 7137, 7432, 7657, 7757, 7805, 8450, 8639, 9560, 9897, 9958, 10237, 10602, 10897, 11453, 11966, 12357, 13997, 14500, 14928, 15208, 15246, 15662, 16548, 17534, 17646, 20156, 20255, 20472, 20558, 20952, 20962, 21727, 22107, 22359, 23709, 23873, 24263, 24270, 24907, 24942, 25083, 25589, 25736, 26709, 27276, 27303, 28117, 28423, 28597, 28813, 29368, 29468, 29562, 29606, 30372, 30405, 31137, 31151}; // STC4A lpGBT:1
  //uint64_t refEvt[14] = {5152, 8783, 10290, 18288, 18458, 19709, 20780, 23286, 24086, 26662, 26972, 28046, 28504, 30599};
  std::vector<uint64_t> refEvents;
  for(int ievent=0;ievent<14;ievent++) refEvents.push_back(refEvt[ievent]);
  refEvents.resize(0);
  //===============================================================================================================================

  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>> hrocarray; //event,rocId
  std::map<uint64_t,uint32_t> eventbx;
  std::vector<uint64_t> eventList;
  uint64_t minEventDAQ = 0, maxEventDAQ = 100;
  
  hrocarray.clear();
  eventList.clear();
  econtarray.clear();
  
  econTReader.init(relayNumber,runNumber,0);
  std::cout<<"TRIG Before Link "<<trig_linkNumber<<", size : " << econtarray.size() <<std::endl;
  econTReader.getEvents(refEvents, minEventDAQ, maxEventDAQ, econtarray);
  std::cout<<"TRIG After Link "<<trig_linkNumber<<", size : " << econtarray.size() <<std::endl;
  econTReader.terminate();
  
  econDReader.init(relayNumber,runNumber,linkNumber);
  //econDReader.setModulePath(zside, sector, link, det, econt, selTC4, module);
  econDReader.getEvents(refEvents, minEventDAQ, maxEventDAQ, hrocarray, eventList); //Input <---> output
  econDReader.terminate();
  
  //for(const auto& ievent : eventList){
  std::vector<uint64_t> totEvents;
  for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
    uint64_t ievent = eventList[ievt] ;
    std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> datalist = hrocarray[ievent] ;
    std::cout << "ievent: "<< ievent<< ", datalist size: "<< datalist.size() << std::endl;
    bool hasTOT = false;
    for(const auto& hrocit : datalist){
      const TPGFEDataformat::HalfHgcrocData& hrocdata = hrocit.second ;
      if(testmodid==pck.getModIdFromRocId(uint32_t(hrocit.first))){
      //if(ievent==3){
	std::cout << "ievent: "<< ievent<< "\t data for half-roc_id: "<< hrocit.first <<  std::endl;
	hrocdata.print();
	if(hrocdata.hasTOT()) hasTOT = true;
      }
    }
    if(hasTOT) totEvents.push_back(ievent);
    // std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>> econtdata =  econtarray[ievent]; 
    // for(const auto& econtit : econtdata){
    //   //std::cout << "\t data for econt_id: "<< econtit.first <<  std::endl;
    //   TPGBEDataformat::Trig24Data trdata = econtit.second ;
    //   //trdata.print();
    // }
  }
  if(totEvents.size()>0) std::cerr<< "uint64_t refEvt["<< totEvents.size() <<"] = {";
  for(const uint64_t& totEvt : totEvents) std::cerr<<totEvt << ", ";
  if(totEvents.size()>0) std::cerr<< "};" << std::endl;
  
  //===============================================================================================================================
  // Emulate
  //===============================================================================================================================
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>> modarray; //event,moduleId
  modarray.clear();
  std::map<uint32_t,TPGFEDataformat::HalfHgcrocData> rocdata;
  std::map<uint32_t,TPGFEDataformat::ModuleTcData> moddata;
  /////////////////////////////////////////////////////////////
  //// The following part should be in a loop over modules
  /////////////////////////////////////////////////////////////
  zside = 0, sector = 0, link = 0, det = 0;
  econt = 0, selTC4 = 1, module = 0;
  uint32_t moduleId = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  moduleId = testmodid;
  std::cout<<"modarray : Before Link"<<linkNumber<<" size : " << modarray.size() << ", modId : "<< moduleId <<std::endl;
  std::cout<<"eventList size: " << eventList.size() <<std::endl;
  
  for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
    uint64_t event = eventList[ievt] ;

    //first check that both econd and econt data has the same eventid otherwise skip the event
    if(econtarray.find(event) == econtarray.end() or hrocarray.find(event) == hrocarray.end()) break;

    std::cout<<std::endl<<std::endl<<"=========================================================================="<<std::endl<<"Processing event: " << event <<std::endl<<std::endl<<std::endl;
    
    std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata; 
    //;
    
    rocdata.clear();
    int bx, bx4;
    for(const auto& data : hrocarray.at(event)){
      rocdata[data.first] = data.second ;
      if(data.first==moduleId){
	bx = rocdata[data.first].getBx();
	bx4 = bx & 0xF;
      }
    }
      
    bool isSim = false; //true for CMSSW simulation and false for beam-test analysis
    rocTPGEmul.Emulate(isSim, event, moduleId, rocdata, modTcdata);
    
    modarray[event].push_back(modTcdata);

    if(event==1){
      uint32_t modTcid = modTcdata.first;
      TPGFEDataformat::ModuleTcData& mtcdata = modTcdata.second;
      if(modTcid==moduleId) mtcdata.print();
    }
    
    moddata.clear();
    for(const auto& data : modarray.at(event))
      moddata[data.first] = data.second ;
    
    econtEmul.Emulate(isSim, event, moduleId, moddata);
    TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
    
    // if(event==0){
    //   TcRawdata.print();
    // }
    
    std::cout << "Emul: ievent: "<< event << ", bx: " << bx <<", bx4: "<< bx4 << std::endl;
    std::cout<<"Module :: "<<TcRawdata.first << ", refmodule : " << moduleId << std::endl;
    if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
    TcRawdata.second.print();
    
    uint32_t elinkemul[6];
    TPGFEModuleEmulation::ECONTEmulation::convertToElinkData(bx4, TcRawdata.second, elinkemul);
    std::cout << "Emulation: ievent: "<< event << std::endl;
    std::cout<<"Elink emul : 0x" << std::hex << ::std::setfill('0') << std::setw(8) << elinkemul[0] << std::dec << std::endl;
    std::cout<<"           : 0x" << std::hex << ::std::setfill('0') << std::setw(8) << elinkemul[1] << std::dec << std::endl;
    std::cout<<"           : 0x" << std::hex << ::std::setfill('0') << std::setw(8) << elinkemul[2] << std::dec << std::endl;
    
    std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>> econtdata =  econtarray[event];
    
    for(const auto& econtit : econtdata){
      if(econtit.first!=TcRawdata.first) continue;
      TPGBEDataformat::Trig24Data trdata = econtit.second ;
      std::cout << "ECONT: ievent: "<< event << std::endl;
      std::cout << "\t data for econt_id: "<< econtit.first <<  std::endl;
      TPGFEDataformat::TcRawDataPacket vTC1, vTC2, vTC3;
      TPGBEDataformat::UnpackerOutputStreamPair up1, up2,up3;
      const uint32_t *el = trdata.getElinks(2); //5 for BC and 3 for STC4A and CTC4A , 6 for BC of lpGBT:0
      uint16_t bx_2 = (el[0]>>28) & 0xF ;
      TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, 6, el, vTC1);
      trdata.setNofUnpkWords(6);
      TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(bx_2, vTC1, up1);      
      vTC1.print();
      up1.print();
      trdata.print();
    }
    
  }//event loop
  //===============================================================================================================================

  
  return true;
}
