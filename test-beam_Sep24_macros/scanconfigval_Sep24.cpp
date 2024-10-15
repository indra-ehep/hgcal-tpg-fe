/**********************************************************************
 Created on : 16/09/2024
 Purpose    : Emulation for test-beam of September 2024
 Author     : Indranil Das, Research Associate
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
  //===============================================================================================================================
  // make
  // ./emul_Jul24.exe $Relay $rname $link_number $istrimming $density $dropLSB 
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
  const uint32_t noflpGBTs(2);
  bool isTrimming(0);
  uint32_t density(0);
  uint32_t droplsb(1);
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
  std::istringstream issTrimming(argv[4]);
  issTrimming >> isTrimming;
  std::istringstream issDensity(argv[5]);
  issDensity >> density;
  std::istringstream issDropLSB(argv[6]);
  issDropLSB >> droplsb;
  //===============================================================================================================================

  //===============================================================================================================================
  //Book histograms
  //===============================================================================================================================
  void BookHistograms(TDirectory*&, uint32_t);
  void FillHistogram(bool, bool, TDirectory*&, uint32_t, uint64_t, uint16_t, uint32_t, const uint32_t *, const uint32_t *, uint32_t, uint32_t,
		     TPGFEDataformat::TcRawDataPacket&, TPGFEDataformat::TcRawDataPacket&, uint32_t, uint32_t);
  
  //FillHistogram(true, isLargeDiff, dir_diff, relayNumber, event, econt_slink_bx, econTPar[moduleId].getNElinks(), eldata, elinkemul, ilink, iecond, vTC1, TcRawdata.second, itotp, itoth);
  
  TFile *fout = new TFile(Form("Diff_Relay-%u.root",relayNumber), "recreate");
  TDirectory *dir_diff = fout->mkdir("diff_plots");
  BookHistograms(dir_diff, relayNumber);
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
  
  std::string cfgrocname[2][3][3] = { //nof lpGBTs, nofModules, nofHGCROC per modules
    {
      {"320MLF3WXIH0019_roc0_e2.yaml", "320MLF3WXIH0019_roc1_e2.yaml", "320MLF3WXIH0019_roc2_e2.yaml"},
      {"320MLF3WXIH0020_roc0_e1.yaml", "320MLF3WXIH0020_roc1_e1.yaml", "320MLF3WXIH0020_roc2_e1.yaml"},
      {"320MLF3WXIH0018_roc0_e0.yaml", "320MLF3WXIH0018_roc1_e0.yaml", "320MLF3WXIH0018_roc2_e0.yaml"}
    },
    {
      {"320MLF3WXIH0016_roc0_e2.yaml", "320MLF3WXIH0016_roc1_e2.yaml", "320MLF3WXIH0016_roc2_e2.yaml"},
      {"320MLF3WXIH0017_roc0_e1.yaml", "320MLF3WXIH0017_roc1_e1.yaml", "320MLF3WXIH0017_roc2_e1.yaml"},
      {"320MLF3WXIH0014_roc0_e0.yaml", "320MLF3WXIH0014_roc1_e0.yaml", "320MLF3WXIH0014_roc2_e0.yaml"}
    }
  };

  // std::string cfgrocname[2][3][3] = { //nof lpGBTs, nofModules, nofHGCROC per modules
  //   {
  //    {"320MLF3WXIH0019_roc0_e2.yaml", "320MLF3WXIH0019_roc1_e2.yaml", "320MLF3WXIH0019_roc2_e2.yaml"},
  //    {"320MLF3WXIH0020_roc0_e1.yaml", "320MLF3WXIH0020_roc1_e1.yaml", "320MLF3WXIH0020_roc2_e1.yaml"},
  //    {"320MLF3WXIH0018_roc0_e0.yaml", "320MLF3WXIH0018_roc1_e0.yaml", "320MLF3WXIH0018_roc2_e0.yaml"}
  //   },
  //   {
  //    {"320MLF3WXIH0016_roc0_e2.yaml", "320MLF3WXIH0016_roc1_e2.yaml", "320MLF3WXIH0016_roc2_e2.yaml"},
  //    {"320MLF3WXIH0017_roc0_e1.yaml", "320MLF3WXIH0017_roc1_e1.yaml", "320MLF3WXIH0017_roc2_e1.yaml"},
  //    {"320MLF3WXIH0014_roc0_e0.yaml", "320MLF3WXIH0014_roc1_e0.yaml", "320MLF3WXIH0014_roc2_e0.yaml"}
  //   }
  // };

  for(int ilink=0;ilink<noflpGBTs;ilink++){
    for(int iecond=0;iecond<3;iecond++){
      uint32_t idx = pck.packModId(zside, sector, ilink, det, iecond, selTC4, module); //we assume same ECONT and ECOND number for a given module
      
      cfgs.setModulePath(zside, sector, ilink, det, iecond, selTC4, module);

      switch(ilink){
      case 0:
	switch(iecond){
	case 0:
	  cfgs.setEconTFile("cfgmap/init_econt_e2.yaml");
	  break;
	case 1:
	  cfgs.setEconTFile("cfgmap/init_econt_e1.yaml");
	  break;
	default:
	  cfgs.setEconTFile("cfgmap/init_econt.yaml");
	  break;
	}
      case 1:
	switch(iecond){
	case 0:
	  cfgs.setEconTFile("cfgmap/init_econt_e2.yaml");
	  break;
	case 1:
	  cfgs.setEconTFile("cfgmap/init_econt_e1.yaml");
	  break;
	default:
	  cfgs.setEconTFile("cfgmap/init_econt.yaml");
	  break;
	}
	default:
	  cfgs.setEconTFile("cfgmap/init_econt.yaml");
	  break;
      }
      cfgs.readEconTConfigYaml();

      // if(ilink==1 and (iecond==0)){
      // 	cfgs.setEconTFile("cfgmap/init_econt_mux_test1.yaml");
      // 	cfgs.readEconTConfigYaml();
      // }
      
      cfgs.setEconTFile("cfgmap/init_econt_mux_test1.yaml");
      cfgs.readEconTConfigYaml();

      cfgs.setEconDFile("cfgmap/init_econd.yaml");
      cfgs.readEconDConfigYaml();      
      
      for(uint32_t iroc=0;iroc<3;iroc++){
	std::cout<<"idx: "<<idx<<", ilink: " << ilink << ", iecond: "<<iecond<<", iroc: "<<iroc << ", fname : " << cfgrocname[ilink][iecond][iroc] << std::endl;
	uint32_t rocid_0 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 0);
	uint32_t rocid_1 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 1);
	//cfgs.setRocFile(Form("cfgmap/slow_control_configuration/configs_with_MasterTDC_and_TPGTRM/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	//cfgs.setRocFile(Form("cfgmap/bt2024-fe-config-trimmed/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	cfgs.setRocFile(Form("cfgmap/configs_v3b_full/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	//cfgs.setRocFile(Form("cfgmap/configs_v3b_full_aggressive/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	cfgs.readRocConfigYaml(rocid_0, rocid_1);	
      }//roc loop
    }//econd loop
  }//lpGBT loop
  if(!isTrimming){
    if(relayNumber==1726581356)
      cfgs.setPedZero();
    else
      cfgs.setPedThZero();
  }
  link = 1; econt = 2;
  uint32_t testmodid = pck.packModId(zside, sector, link, det, econt, selTC4, module); //we assume same ECONT and ECOND number for a given module
  cfgs.printCfgPedTh(testmodid);
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Modify the ECON parameters for special cases
  //===============================================================================================================================
  std::map<uint32_t,TPGFEConfiguration::ConfigEconD>& econDPar =  cfgs.getEconDPara();
  for(const auto& it : econDPar){
    //std::cout << "it.first: "<< it.first << std::endl;
    econDPar.at(it.first).setPassThrough(true);
    econDPar.at(it.first).setNeRx(6);
  }
  link = 0; econt = 0;
  uint32_t lp0_bc0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 0; econt = 1;
  uint32_t lp0_bc1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 0; econt = 2;
  uint32_t lp0_bc2 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 0;
  uint32_t lp1_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 1;
  uint32_t lp1_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 2;
  uint32_t lp1_stc4a2 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar =  cfgs.getEconTPara();
  for(const auto& it : econTPar){
    econTPar[it.first].setDensity(density);
    econTPar[it.first].setDropLSB(droplsb);
    
    if(it.first==lp0_bc0){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(3);      
    }else if (it.first==lp0_bc1){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(2);     
    }else if (it.first==lp0_bc2){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(2);
    }else if (it.first==lp1_stc4a0){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(3);
      econTPar[it.first].setSTCType(3);
      if(relayNumber>=1727111828){ //1727211141
	econTPar[it.first].setNElinks(2);
	econTPar[it.first].setSTCType(1);
      }
    }else if (it.first==lp1_stc4a1){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(3);
      if(relayNumber>=1727111828){ //1727211141
	econTPar[it.first].setNElinks(2);
	econTPar[it.first].setSTCType(1);
      }
    }else if (it.first==lp1_stc4a2){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(3);
      if(relayNumber>=1727111828){ //1727211141
	econTPar[it.first].setNElinks(2);
	econTPar[it.first].setSTCType(1);
      }
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
  
  //===============================================================================================================================
  //Modify the HGCROC parameters for special cases
  //===============================================================================================================================
  // std::map<uint64_t,TPGFEConfiguration::ConfigCh>& hrocchcfg =  cfgs.getChPara();
  link = 1; econt = 0; uint32_t roc = 2, hroc = 0;
  uint32_t rocid_0 = pck.packRocId(zside, sector, link, det, econt, selTC4, module, roc, hroc);  
  std::map<uint32_t,TPGFEConfiguration::ConfigHfROC>& hroccfg =  cfgs.getRocPara();
  for(auto const& hroc : hroccfg){
    if(hroc.first == rocid_0) {
      hroccfg[hroc.first].setTotP(2, 13);
      hroccfg[hroc.first].setTotP(3, 0);
      hroccfg[hroc.first].setTotTH(2, 107);
    }
  }
  // ===============================================================================================================================
  
  // ===============================================================================================================================
  // Set and Initialize the ECONT reader
  // ===============================================================================================================================
  TPGFEReader::ECONTReader econTReader(cfgs);
  //output array
  //econTReader.checkEvent(1);
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>>> econtarray; //event,moduleId (from link)
  // ===============================================================================================================================
  
  //===============================================================================================================================
  //Set and Initialize the ECOND reader
  //===============================================================================================================================
  TPGFEReader::ECONDReader econDReader(cfgs);
  // econDReader.setTotUp(0);
  // econDReader.checkEvent(1);
  // econDReader.showFirstEvents(10);
  //===============================================================================================================================
  //Set and Initialize the Emulator
  //===============================================================================================================================
  TPGFEModuleEmulation::HGCROCTPGEmulation rocTPGEmul(cfgs);
  TPGFEModuleEmulation::ECONTEmulation econtEmul(cfgs);
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Set refernce eventlist
  //===============================================================================================================================
  // /*TcTp2 events */ uint64_t refEvt[50] = {66235, 127532, 145302, 151731, 194816, 260756, 269698, 383736, 391481, 397781, 471496, 534349, 605680, 613325, 694404, 873524, 919213, 1085378, 1124673, 1164106, 1222670, 1247896, 1278016, 1322821, 1453497, 1486909, 1533310, 1623036, 1661345, 1763515, 1769677, 1809836, 1829961, 1871023, 1932945, 1967257, 1968826, 1981516, 1984675, 2043998, 2060794, 2068723, 2165732, 2172380, 2237980, 2245408, 2259447, 2390648, 2462559, 2514946};
  // /*Nonzero diff events*/ uint64_t refEvt[6] = {66235, 471496, 1763515, 1968826, 2245408, 2390648};
  /*STC 10 events*/ uint64_t refEvt[7] = {66235, 471496, 873524, 1763515, 1968826, 2245408, 2390648};
  
  std::vector<uint64_t> refEvents;
  //for(int ievent=0;ievent<1020;ievent++) refEvents.push_back(refEvt[ievent]);
  for(int ievent=0;ievent<7;ievent++) refEvents.push_back(refEvt[ievent]);
  //refEvents.resize(0);
  //===============================================================================================================================
  
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>> hrocarray; //event,rocId
  std::map<uint64_t,uint32_t> eventbx;
  std::vector<uint64_t> eventList;
  std::vector<uint64_t> nonZeroEvents;
  std::vector<uint64_t> shiftedBxEvent;
  std::vector<uint64_t> corruptEvent;
  std::vector<uint64_t> unExDataTC;
  std::vector<uint64_t> unExEmulTC;
  std::vector<uint64_t> elStg1Event;
  std::vector<uint64_t> TcTp1Events;
  std::vector<uint64_t> TcTp2Events;
  std::vector<uint64_t> TcTp3Events;
  std::vector<uint64_t> TcTp1TrigEvents;
  std::vector<uint64_t> TcTp2TrigEvents;
  std::vector<uint64_t> TcTp3TrigEvents;
  
  uint64_t minEventDAQ, maxEventDAQ;  
  //const long double maxEvent = 1000000 ;
  const long double maxEvent = 2557415 ;     //relay:1726581356
  //const long double maxEvent = 528447 ;         //relay:1727211141
  long double nloopEvent =  100000;
  //const long double maxEvent = 1000  ; //1722870998:24628, 1722871979:31599
  //long double nloopEvent = 1000 ;
  int nloop = TMath::CeilNint(maxEvent/nloopEvent) ;
  if(refEvents.size()>0) nloop = 1;
  std::cout<<"nloop: "<<nloop<<std::endl;
  for(int ieloop=0;ieloop<nloop;ieloop++){
    
    minEventDAQ = ieloop*nloopEvent ;
    maxEventDAQ = (ieloop+1)*nloopEvent;
    maxEventDAQ = (maxEventDAQ>uint64_t(maxEvent)) ? uint64_t(maxEvent) : maxEventDAQ ;
    
    printf("iloop : %d, minEventDAQ = %lu, maxEventDAQ = %lu\n",ieloop,minEventDAQ, maxEventDAQ);
    std::cerr<<"iloop : "<< ieloop <<", minEventDAQ = "<< minEventDAQ <<", maxEventDAQ = " << maxEventDAQ << std::endl;

    hrocarray.clear();
    eventList.clear();
    econtarray.clear();
    
    econTReader.setNofCAFESep(11);
    if(relayNumber>=1727211141) econTReader.setNofCAFESep(14);
    econTReader.init(relayNumber,runNumber,0);
    std::cout<<"TRIG Before Link "<<trig_linkNumber<<", size : " << econtarray.size() <<std::endl;
    econTReader.getEvents(refEvents, minEventDAQ, maxEventDAQ, econtarray);
    std::cout<<"TRIG After Link "<<trig_linkNumber<<", size : " << econtarray.size() <<std::endl;
    econTReader.terminate();
    
    econDReader.init(relayNumber,runNumber,linkNumber);
    std::cout<<"DAQ Before Link "<<linkNumber<<", size : " << hrocarray.size() <<std::endl;
    econDReader.getEvents(refEvents, minEventDAQ, maxEventDAQ, hrocarray, eventList); //Input <---> output
    std::cout<<"DAQ After Link "<<linkNumber<<", size : " << hrocarray.size() <<std::endl;
    econDReader.terminate();
    
    // for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
    //   uint64_t ievent = eventList[ievt] ;
    //   std::cout << "Event: " << ievent << ", ECONT: " << econtarray.at(ievent).size() << ", ROC: "<< hrocarray.at(ievent).size() << std::endl;
    // }
    // continue;
    
    for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
      uint64_t ievent = eventList[ievt] ;
      if(econtarray.at(ievent).size()!=6 or hrocarray.at(ievent).size()!=36) continue;
      std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> datalist = hrocarray[ievent] ;
      if(ievt<10) std::cout << "ROC ievent: "<< ievent<< ", datalist size: "<< datalist.size() << std::endl;
      for(const auto& hrocit : datalist){
	const TPGFEDataformat::HalfHgcrocData& hrocdata = hrocit.second ;
	//if(testmodid==pck.getModIdFromRocId(uint32_t(hrocit.first))){
	//if(event<100 and (iecond==1 and ilink==0)){
	if(ievt<10) std::cout << "ievent: "<< ievent<< "\t data for modid : "<< pck.getModIdFromRocId(uint32_t(hrocit.first)) << ",\t half-roc_id: "<< hrocit.first <<  std::endl;
	if(ievt<10) hrocdata.print();
	if(hrocdata.hasTcTp(1) and (std::find(TcTp1Events.begin(),TcTp1Events.end(),ievent) == TcTp1Events.end())) TcTp1Events.push_back(ievent);
	if(hrocdata.hasTcTp(2) and (std::find(TcTp2Events.begin(),TcTp2Events.end(),ievent) == TcTp2Events.end())) TcTp2Events.push_back(ievent);
	if(hrocdata.hasTcTp(3) and (std::find(TcTp3Events.begin(),TcTp3Events.end(),ievent) == TcTp3Events.end())) TcTp3Events.push_back(ievent);
      }
    }
    
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
    std::cout<<"eventList size: " << eventList.size() <<std::endl;

    for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
      uint64_t event = eventList[ievt] ;
      
      //first check that both econd and econt data has the same eventid otherwise skip the event
      if(econtarray.find(event) == econtarray.end() or hrocarray.find(event) == hrocarray.end() or econtarray.at(event).size()!=6 or hrocarray.at(event).size()!=36) {
	if(std::find(corruptEvent.begin(),corruptEvent.end(),event) == corruptEvent.end()) corruptEvent.push_back(event);
	continue;
      }
      //if(econtarray.find(event) == econtarray.end()) break;

      //bool eventCondn = (event<1000 or event==1560 or event==2232 or event==2584 or event==2968 or event==3992);
      //bool eventCondn = (event<1000);
      bool eventCondn = (refEvents.size()==0) ? (ievt<10) : (ievt<refEvents.size());
      if( eventCondn or event%100000==0)
	std::cout<<std::endl<<std::endl<<"=========================================================================="<<std::endl<<"Processing event: " << event <<std::endl<<std::endl<<std::endl;
      
      for(int ilink=0;ilink<2;ilink++){
	for(int iecond=0;iecond<3;iecond++){
	  uint32_t moduleId = pck.packModId(zside, sector, ilink, det, iecond, selTC4, module); //we assume same ECONT and ECOND number for a given module
	  
	  rocdata.clear();
	  int emul_bx;
	  uint32_t econd_bx, econd_bx_4b, emul_bx_4b;
	  uint16_t econt_slink_bx, econd_slink_bx;
	  
	  std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> hrocvec ;
	  // if(event==1560 or event==2232 or event==2584 or event==2968 or event==3992)
	  //   hrocvec =  hrocarray.at(event+1);
	  // else
	  hrocvec =  hrocarray.at(event);
	  
	  for(const auto& data : hrocvec){
	    rocdata[data.first] = data.second ;
	    const TPGFEDataformat::HalfHgcrocData& hrocdata = data.second ;
	    //if(moduleId==pck.getModIdFromRocId(uint32_t(data.first)))
	    
	    if(data.first==moduleId){
	      econd_bx = rocdata[data.first].getBx();
	      econd_slink_bx = rocdata[data.first].getSlinkBx();
	      emul_bx = econd_bx - 0;
	      if(emul_bx<0) emul_bx = (3564+emul_bx) + 1;
	      emul_bx_4b = (rocdata[data.first].getBx()==3564) ? 0xF : (emul_bx%8);
	      econd_bx_4b = (rocdata[data.first].getBx()==3564) ? 0xF : (econd_bx%8);
	      //tune the bx here to match the TPG
	      if(relayNumber==1726581356)
		if(rocdata[data.first].getBx()==3564)
		  emul_bx_4b = 1;
		else if(rocdata[data.first].getBx()==3563)
		  emul_bx_4b = 0xF;
		else if(emul_bx_4b==7)
		  emul_bx_4b = 0x0;
		else
		  emul_bx_4b += 1;	      
	      if(relayNumber==1726435152) emul_bx_4b += 0;
	      if(relayNumber==1722698259) emul_bx_4b += 7;
	      if(relayNumber==1723627575) emul_bx_4b += 2;
	      if(relayNumber==1722702638) emul_bx_4b += 7;
	      if(relayNumber==1722870998) emul_bx_4b += 2;
	      if(relayNumber==1722871974) emul_bx_4b += 2;
	    }
	  }
	  
	  std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>> econtdata =  econtarray[event];	  
	  int refbx = -1;
	  TPGBEDataformat::Trig24Data trdata;
	  for(const auto& econtit : econtdata){
	    if(econtit.first!=moduleId) continue;
	    trdata = econtit.second ;
	    if(eventCondn) std::cout << "event: " << event << ", moduleId : " << econtit.first << std::endl;
	    //if(eventCondn and iecond==0 and ilink==0) TcRawdata.second.print();
	    bool hasMatched = false;
	    for(int ibx=0;ibx<7;ibx++){
	      const uint32_t *el = trdata.getElinks(ibx); 
	      uint32_t bx_2 = (el[0]>>28) & 0xF;
	      if(bx_2==emul_bx_4b) {
		refbx = ibx;
		hasMatched = true;
	      }//match found;
	      if(hasMatched) break;
	    }//bx loop
	  }//loop over econt data for a given run
	  if(refbx==-1){
	    //std::cout << "refbx: " << refbx << std::endl ;
	    if(std::find(shiftedBxEvent.begin(),shiftedBxEvent.end(),event) == shiftedBxEvent.end()) shiftedBxEvent.push_back(event);
	    econt_slink_bx = trdata.getSlinkBx();
	    std::cout<<std::endl<<std::endl<<"=========================================================================="<<std::endl<<"Skipping event: " << event <<std::endl<<std::endl<<std::endl;
	    std::cout << "Skip Event: "<< event
		      // << ", ECOND:: (econd_slink_bx: " << econd_slink_bx <<", econd_bx: "<<econd_bx<<", econd econd_bx_4b : " <<  econd_bx_4b
		      // << ") ECONT:: (econt_slink_bx: " << trdata.getSlinkBx() <<", econt_slink_bx%8: "<< (trdata.getSlinkBx()%8) <<", TPG bxid range : [" << tpg_m3_bxid << ", " << tpg_p3_bxid << "] "
		      <<"), moduleId : " << moduleId << std::endl;
	    continue;
	  }
	  const uint32_t *eldata = trdata.getElinks(refbx);
	  econt_slink_bx = trdata.getSlinkBx();
	  TPGFEDataformat::TcRawDataPacket vTC1;
	  if(iecond==0 and ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, 6, eldata, vTC1);
	  if(iecond==1 and ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, 4, eldata, vTC1);
	  if(iecond==2 and ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, 4, eldata, vTC1);
	  if(relayNumber>=1727111828){
	    if(iecond==0 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, 3, eldata, vTC1);
	    if(iecond==1 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, 3, eldata, vTC1);
	    if(iecond==2 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, 3, eldata, vTC1);
	  }else{
	    if(iecond==0 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, 10, eldata, vTC1);
	    if(iecond==1 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, 6, eldata, vTC1);
	    if(iecond==2 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, 6, eldata, vTC1);
	  }
	  
	  for(uint32_t itoth=0; itoth<200 and ilink==1 and iecond==0; itoth++){
	    for(uint32_t itotp=0; itotp<100; itotp++){
	      if(event==2245408){	     
		hroccfg[rocid_0].setTotP(3, itotp);
		hroccfg[rocid_0].setTotTH(3, itoth);
	      }else{
		hroccfg[rocid_0].setTotP(2, itotp);
		hroccfg[rocid_0].setTotTH(2, itoth);
	      }
	      std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	      //================================================
	      //HGCROC emulation for a given module
	      //================================================
	      bool isSim = false; //true for CMSSW simulation and false for beam-test analysis
	      rocTPGEmul.Emulate(isSim, event, moduleId, rocdata, modTcdata);
	      //================================================
	  
	      modarray[event].push_back(modTcdata);
	      if(event==0){
		uint32_t modTcid = modTcdata.first;
		TPGFEDataformat::ModuleTcData& mtcdata = modTcdata.second;
		if(modTcid==moduleId) mtcdata.print();
	      }
	  
	      moddata.clear();
	      for(const auto& data : modarray.at(event))
		if(data.first==moduleId) moddata[data.first] = data.second ;
	      
	      // if(ilink==1 and iecond==0)
	      //   econtEmul.setVerbose();
	      // else
	      //   econtEmul.setVerbose(false);
	      
	      //================================================
	      //ECONT emulation for a given module
	      //================================================
	      econtEmul.Emulate(isSim, event, moduleId, moddata);
	      TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	      //================================================
	      if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	      TcRawdata.second.setBX(emul_bx_4b);
	      
	      uint32_t *elinkemul = new uint32_t[econTPar[moduleId].getNElinks()];
	      TPGFEModuleEmulation::ECONTEmulation::convertToElinkData(emul_bx_4b, TcRawdata.second, elinkemul);

	      if(TcRawdata.second.isTcTp1() and (std::find(TcTp1TrigEvents.begin(),TcTp1TrigEvents.end(),event) == TcTp1TrigEvents.end())) TcTp1TrigEvents.push_back(event);
	      if(TcRawdata.second.isTcTp2() and (std::find(TcTp2TrigEvents.begin(),TcTp2TrigEvents.end(),event) == TcTp2TrigEvents.end())) TcTp2TrigEvents.push_back(event);
	      if(TcRawdata.second.isTcTp3() and (std::find(TcTp3TrigEvents.begin(),TcTp3TrigEvents.end(),event) == TcTp3TrigEvents.end())) TcTp3TrigEvents.push_back(event);
	      
	      // for(uint32_t iel=0;iel<econTPar[moduleId].getNElinks();iel++){
	      // 	if(printCondn) std::cout<<"Elink emul : 0x" << std::hex << std::setfill('0') << std::setw(8) << elinkemul[iel] << std::dec ;
	      // 	if(printCondn) std::cout<<"\t Elink data : 0x" << std::hex << std::setfill('0') << std::setw(8) << eldata[iel] << std::dec ;//<< std::endl;
	      // 	if(printCondn) std::cout<<"\t Diff: " << std::setfill(' ') << std::setw(10) << (eldata[iel]-elinkemul[iel])  << ", XOR: " << std::bitset<32>{(eldata[iel] ^ elinkemul[iel])} << std::endl;
	      // 	if( ((eldata[iel]-elinkemul[iel]) != 0) and (std::find(nonZeroEvents.begin(),nonZeroEvents.end(),event) == nonZeroEvents.end())) nonZeroEvents.push_back(event);
	      // }
	      bool isLargeDiff = false;
	      for(uint32_t iel=0;iel<econTPar[moduleId].getNElinks();iel++){
		if((eldata[iel]-elinkemul[iel]) != 0) isLargeDiff = true;
	      }
	      
	      FillHistogram(true, isLargeDiff, dir_diff, relayNumber, event, econt_slink_bx, econTPar[moduleId].getNElinks(), eldata, elinkemul, ilink, iecond, vTC1, TcRawdata.second, itotp, itoth);
	      delete []elinkemul;
	      
	    }//totp loop
	  }//toth loop
	}//econd loop
      }//ilink loop    
    }//event loop
  }//ieloop
  //===============================================================================================================================
  if(TcTp1Events.size()>0) std::cerr<< "/*TcTp1 events */ uint64_t refEvt["<< TcTp1Events.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp1Events) std::cerr<<totEvt << ", ";
  if(TcTp1Events.size()>0) std::cerr<< "};" << std::endl;

  if(TcTp2Events.size()>0) std::cerr<< "/*TcTp2 events */ uint64_t refEvt["<< TcTp2Events.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp2Events) std::cerr<<totEvt << ", ";
  if(TcTp2Events.size()>0) std::cerr<< "};" << std::endl;

  if(TcTp3Events.size()>0) std::cerr<< "/*TcTp3 events */ uint64_t refEvt["<< TcTp3Events.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp3Events) std::cerr<<totEvt << ", ";
  if(TcTp3Events.size()>0) std::cerr<< "};" << std::endl;

  if(TcTp1TrigEvents.size()>0) std::cerr<< "/*TcTp1 Trig events */ uint64_t refEvt["<< TcTp1TrigEvents.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp1TrigEvents) std::cerr<<totEvt << ", ";
  if(TcTp1TrigEvents.size()>0) std::cerr<< "};" << std::endl;

  if(TcTp2TrigEvents.size()>0) std::cerr<< "/*TcTp2 Trig events */ uint64_t refEvt["<< TcTp2TrigEvents.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp2TrigEvents) std::cerr<<totEvt << ", ";
  if(TcTp2TrigEvents.size()>0) std::cerr<< "};" << std::endl;

  if(TcTp3TrigEvents.size()>0) std::cerr<< "/*TcTp3 Trig events */ uint64_t refEvt["<< TcTp3TrigEvents.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp3TrigEvents) std::cerr<<totEvt << ", ";
  if(TcTp3TrigEvents.size()>0) std::cerr<< "};" << std::endl;
  
  // if(nonZeroEvents.size()>0) std::cerr<< "/*Nonzero diff events*/ uint64_t refEvt["<< nonZeroEvents.size() <<"] = {";
  // for(const uint64_t& totEvt : nonZeroEvents) std::cerr<<totEvt << ", ";
  // if(nonZeroEvents.size()>0) std::cerr<< "};" << std::endl;


  fout->cd();
  dir_diff->Write();
  fout->Close();
  delete fout;
  
  return true;
}


void BookHistograms(TDirectory*& dir_diff, uint32_t relay){
  
  TH1D *hElinkDiff = new TH1D("hElinkDiff", Form("Elink diff for Relay: %u",relay), 1000, -10., 10.);
  hElinkDiff->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hElinkDiff->GetYaxis()->SetTitle("Entries");
  hElinkDiff->SetDirectory(dir_diff);
  
  TH1D *hElinkDiff_0 = new TH1D("hElinkDiff_0", Form("Elink diff for Relay: %u, for tctp case : 0",relay), 1000, -10., 10.);
  hElinkDiff_0->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hElinkDiff_0->GetYaxis()->SetTitle("Entries");
  hElinkDiff_0->SetDirectory(dir_diff);

  TH1D *hElinkDiff_1 = new TH1D("hElinkDiff_1", Form("Elink diff for Relay: %u, for tctp case : 1",relay), 1000, -10., 10.);
  hElinkDiff_1->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hElinkDiff_1->GetYaxis()->SetTitle("Entries");
  hElinkDiff_1->SetDirectory(dir_diff);

  TH1D *hElinkDiff_2 = new TH1D("hElinkDiff_2", Form("Elink diff for Relay: %u, for tctp case : 2",relay), 1000, -10., 10.);
  hElinkDiff_2->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hElinkDiff_2->GetYaxis()->SetTitle("Entries");
  hElinkDiff_2->SetDirectory(dir_diff);

  TH1D *hElinkDiff_3 = new TH1D("hElinkDiff_3", Form("Elink diff for Relay: %u, for tctp case : 3",relay), 1000, -10., 10.);
  hElinkDiff_3->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hElinkDiff_3->GetYaxis()->SetTitle("Entries");
  hElinkDiff_3->SetDirectory(dir_diff);
  
  TH2D *hTotPvsTotTH_n2 = new TH2D("hTotPvsTotTH_n2", Form("TotP-vs-TotTH for n==2 of Relay: %u",relay), 100, -0.5, 99.5, 200, -0.5, 199.5);
  hTotPvsTotTH_n2->GetXaxis()->SetTitle("Tot_P(n==2)");
  hTotPvsTotTH_n2->GetYaxis()->SetTitle("Tot_TH(n==2)");
  hTotPvsTotTH_n2->SetDirectory(dir_diff);

  TH2D *hTotPvsTotTH_n3 = new TH2D("hTotPvsTotTH_n3", Form("TotP-vs-TotTH for n==3 of Relay: %u",relay), 100, -0.5, 99.5, 200, -0.5, 199.5);
  hTotPvsTotTH_n3->GetXaxis()->SetTitle("Tot_P(n==3)");
  hTotPvsTotTH_n3->GetYaxis()->SetTitle("Tot_TH(n==3)");
  hTotPvsTotTH_n3->SetDirectory(dir_diff);

  uint64_t refEvt[7] = {66235, 471496, 873524, 1763515, 1968826, 2245408, 2390648};
  TH2D *hTotPvsTotTH_Event[7];
  for(int ievent=0;ievent<7;ievent++){
    if(refEvt[ievent]==2245408)
      hTotPvsTotTH_Event[ievent] = new TH2D(Form("hTotPvsTotTH_Event_%llu",refEvt[ievent]), Form("TotP-vs-TotTH for n==3 of Relay: %u and Event : %llu",relay,refEvt[ievent]), 100, -0.5, 99.5, 200, -0.5, 199.5);
    else
      hTotPvsTotTH_Event[ievent] = new TH2D(Form("hTotPvsTotTH_Event_%llu",refEvt[ievent]), Form("TotP-vs-TotTH for n==2 of Relay: %u and Event : %llu",relay,refEvt[ievent]), 100, -0.5, 99.5, 200, -0.5, 199.5);
    hTotPvsTotTH_Event[ievent]->GetXaxis()->SetTitle("Tot_P");
    hTotPvsTotTH_Event[ievent]->GetYaxis()->SetTitle("Tot_TH");
    hTotPvsTotTH_Event[ievent]->SetDirectory(dir_diff);
  }
  
}

void FillHistogram(bool matchFound, bool isLargeDiff, TDirectory*& dir_diff, uint32_t relayNumber, uint64_t event, uint16_t inputbxId,
		   uint32_t nelinks,  const uint32_t *eldata, const uint32_t *elemul,
		   uint32_t ilp, uint32_t imdl,
		   TPGFEDataformat::TcRawDataPacket& tcdata, TPGFEDataformat::TcRawDataPacket& tcemul,
		   uint32_t itotp, uint32_t itoth){
  
  TList *list = (TList *)dir_diff->GetList();
  
  if(matchFound){
    if(isLargeDiff){
      if(event==2245408)
	((TH2D *) list->FindObject("hTotPvsTotTH_n3"))->Fill( itotp, itoth );
      else
	((TH2D *) list->FindObject("hTotPvsTotTH_n2"))->Fill( itotp, itoth );
      ((TH2D *) list->FindObject(Form("hTotPvsTotTH_Event_%llu",event)))->Fill( itotp, itoth );
    }
  }//if matched
  
}

