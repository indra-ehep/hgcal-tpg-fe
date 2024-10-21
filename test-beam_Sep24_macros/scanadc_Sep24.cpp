/**********************************************************************
 Created on : 20/09/2024
 Purpose    : Scan ADC values to find agreement with the data
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
  if(relayNumber>=1727007636){
    std::cerr << "Relaynumber should corresponds to those with two silicon layers configuration" << std::endl;
    return false;
  }
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
  void FillHistogram(TDirectory*& dir, uint32_t relay, uint64_t event, uint16_t bx, uint32_t itc, uint32_t nofpara, uint32_t hist_index, uint32_t input_adc, uint32_t tc_address, uint32_t tc_energy_data, uint32_t tc_energy_emul);


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
      
      cfgs.setEconTFile("cfgmap/init_econt_mux_test1.yaml");
      cfgs.readEconTConfigYaml();

      cfgs.setEconDFile("cfgmap/init_econd.yaml");
      cfgs.readEconDConfigYaml();      
      
      for(uint32_t iroc=0;iroc<3;iroc++){
	std::cout<<"idx: "<<idx<<", ilink: " << ilink << ", iecond: "<<iecond<<", iroc: "<<iroc << ", fname : " << cfgrocname[ilink][iecond][iroc] << std::endl;
	uint32_t rocid_0 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 0);
	uint32_t rocid_1 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 1);
	cfgs.setRocFile(Form("cfgmap/configs_v3b_full/%s",cfgrocname[ilink][iecond][iroc].c_str()));
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
  link = 0; econt = 0;
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
    }else if (it.first==lp1_stc4a1){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp1_stc4a2){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
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
  
  //===============================================================================================================================
  //Modify the HGCROC parameters for special cases
  //===============================================================================================================================
  // std::map<uint64_t,TPGFEConfiguration::ConfigCh>& hrocchcfg =  cfgs.getChPara();
  link = 1; econt = 0; uint32_t roc = 2, hroc = 0;
  uint32_t rocid_0 = pck.packRocId(zside, sector, link, det, econt, selTC4, module, roc, hroc);  
  std::map<uint32_t,TPGFEConfiguration::ConfigHfROC>& hroccfg =  cfgs.getRocPara();
  for(auto const& hroc : hroccfg){
    if(hroc.first == rocid_0) {
      // hroccfg[hroc.first].setTotP(2, 13);
      // hroccfg[hroc.first].setTotP(3, 0);
      // hroccfg[hroc.first].setTotTH(2, 107);
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
  ///*STC 10 events*/ uint64_t refEvt[7] = {66235, 471496, 873524, 1763515, 1968826, 2245408, 2390648};
  /*TcTp1 events */ uint64_t refEvt[2] = {343240, 434333};
  std::vector<uint64_t> refEvents;
  //for(int ievent=0;ievent<1020;ievent++) refEvents.push_back(refEvt[ievent]);
  for(int ievent=0;ievent<2;ievent++) refEvents.push_back(refEvt[ievent]);
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
  const long double maxEvent = 2557415 ;     //relay:1726581356
  long double nloopEvent =  100000;
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
      if(econtarray.at(ievent).size()!=12 or hrocarray.at(ievent).size()!=36) continue;
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
      if(econtarray.find(event) == econtarray.end() or hrocarray.find(event) == hrocarray.end() or econtarray.at(event).size()!=12 or hrocarray.at(event).size()!=36) {
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
	      //if(hasMatched) break;
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
	  if(ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, econTPar[moduleId].getNofTCs(), eldata, vTC1);
	  if(ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, econTPar[moduleId].getNofSTCs(), eldata, vTC1);

	  if(event==434333 and ilink==0 and iecond==0){

	    vTC1.print();
	    pck.setModId(moduleId);    
	    const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = cfgs.getModIdxToName();
	    const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));
	    const std::map<std::pair<std::string,uint32_t>,std::vector<uint32_t>>& tcPinMap = (pck.getDetType()==0)?cfgs.getSiTCToROCpin():cfgs.getSciTCToROCpin();
	    
	    for(uint32_t itc=0; itc<vTC1.size(); itc++){
	      uint32_t tc_address = vTC1.getTc(itc).address();
	      uint16_t tc_energy_data = vTC1.getTc(itc).energy();	      
	      const std::vector<uint32_t>& pinlist = tcPinMap.at(std::make_pair(modName,tc_address)) ;
	      uint32_t *rocid = new uint32_t[pinlist.size()];
	      uint32_t *rocpin = new uint32_t[pinlist.size()];
	      int ich = 0;
	      for(const auto& tcch : pinlist){
	      	rocpin[ich] = tcch%36 ;
	      	uint32_t rocn = TMath::Floor(tcch/72);
	      	uint32_t half = (int(TMath::Floor(tcch/36))%2==0)?0:1;
	      	rocid[ich] = pck.getRocIdFromModId(moduleId,rocn,half);
		ich++;
	      }
	      TPGFEDataformat::HalfHgcrocChannelData& chdata_0 = rocdata.at(rocid[0]).getChannelData(rocpin[0]);
	      TPGFEDataformat::HalfHgcrocData& hrocdata_0 = rocdata.at(rocid[0]);
	      TPGFEDataformat::HalfHgcrocChannelData& chdata_1 = rocdata.at(rocid[1]).getChannelData(rocpin[1]);
	      TPGFEDataformat::HalfHgcrocChannelData& chdata_2 = rocdata.at(rocid[2]).getChannelData(rocpin[2]);
	      TPGFEDataformat::HalfHgcrocChannelData& chdata_3 = rocdata.at(rocid[3]).getChannelData(rocpin[3]);
	      uint16_t ch0_dataadc = chdata_0.getAdc(); uint16_t ch1_dataadc = chdata_1.getAdc();
	      uint16_t ch2_dataadc = chdata_2.getAdc(); uint16_t ch3_dataadc = chdata_3.getAdc();
	      uint16_t ch0_datatctp = chdata_0.getTcTp(); uint16_t ch1_datatctp = chdata_1.getTcTp();
	      uint16_t ch2_datatctp = chdata_2.getTcTp(); uint16_t ch3_datatctp = chdata_3.getTcTp();
	      
	      //============================================================================================
	      //Emulation of 1 free parameter (1-DoF)
	      //============================================================================================
	      ////////////// 1-DOF Single Ch-0 //////////////
	      for(uint16_t adc_ch0=0; adc_ch0<=1023; adc_ch0++){
		chdata_0.setAdc(adc_ch0, ch0_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 1, 0, adc_ch0, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);

	      ////////////// 1-DOF Single Ch-1 //////////////
	      for(uint16_t adc_ch1=0; adc_ch1<=1023; adc_ch1++){
		chdata_1.setAdc(adc_ch1, ch1_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 1, 1, adc_ch1, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);

	      ////////////// 1-DOF Single Ch-2 //////////////
	      for(uint16_t adc_ch2=0; adc_ch2<=1023; adc_ch2++){
		chdata_2.setAdc(adc_ch2, ch2_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 1, 2, adc_ch2, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_2.setAdc(ch2_dataadc, ch2_datatctp);

	      ////////////// 1-DOF Single Ch-3 //////////////
	      for(uint16_t adc_ch3=0; adc_ch3<=1023; adc_ch3++){
		chdata_3.setAdc(adc_ch3, ch3_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 1, 3, adc_ch3, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_3.setAdc(ch3_dataadc, ch3_datatctp);
	      //============================================================================================
	      
	      
	      //============================================================================================
	      // Emulation of 2 free parameters (2-DoF)
	      // In this order {"01", "02", "03", "12", "13", "23"};
	      //============================================================================================
	      ////////////// 2-DOF Double Ch-01 //////////////	      
	      for(uint32_t adc_ch01=0; adc_ch01<=2046; adc_ch01++){
		uint16_t adc_ch0 = TMath::Nint(adc_ch01/2);
		uint16_t adc_ch1 = adc_ch01 - adc_ch0;
		uint32_t totADC = adc_ch0 + adc_ch1;
		chdata_0.setAdc(adc_ch0, ch0_datatctp);
		chdata_1.setAdc(adc_ch1, ch1_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 2, 0, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);

	      ////////////// 2-DOF Double Ch-02 //////////////	      
	      for(uint32_t adc_ch02=0; adc_ch02<=2046; adc_ch02++){
		uint16_t adc_ch0 = TMath::Nint(adc_ch02/2);
		uint16_t adc_ch2 = adc_ch02 - adc_ch0;
		uint32_t totADC = adc_ch0 + adc_ch2;
		chdata_0.setAdc(adc_ch0, ch0_datatctp);
		chdata_2.setAdc(adc_ch2, ch2_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 2, 1, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);
	      chdata_2.setAdc(ch2_dataadc, ch2_datatctp);

	      ////////////// 2-DOF Double Ch-03 //////////////	      
	      for(uint32_t adc_ch03=0; adc_ch03<=2046; adc_ch03++){
		uint16_t adc_ch0 = TMath::Nint(adc_ch03/2);
		uint16_t adc_ch3 = adc_ch03 - adc_ch0;
		uint32_t totADC = adc_ch0 + adc_ch3;
		chdata_0.setAdc(adc_ch0, ch0_datatctp);
		chdata_3.setAdc(adc_ch3, ch3_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 2, 2, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);
	      chdata_3.setAdc(ch3_dataadc, ch3_datatctp);

	      ////////////// 2-DOF Double Ch-12 //////////////	      
	      for(uint32_t adc_ch12=0; adc_ch12<=2046; adc_ch12++){
		uint16_t adc_ch1 = TMath::Nint(adc_ch12/2);
		uint16_t adc_ch2 = adc_ch12 - adc_ch1;
		uint32_t totADC = adc_ch1 + adc_ch2;
		chdata_1.setAdc(adc_ch1, ch1_datatctp);
		chdata_2.setAdc(adc_ch2, ch2_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 2, 3, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);
	      chdata_2.setAdc(ch2_dataadc, ch2_datatctp);

	      ////////////// 2-DOF Double Ch-13 //////////////	      
	      for(uint32_t adc_ch13=0; adc_ch13<=2046; adc_ch13++){
		uint16_t adc_ch1 = TMath::Nint(adc_ch13/2);
		uint16_t adc_ch3 = adc_ch13 - adc_ch1;
		uint32_t totADC = adc_ch1 + adc_ch3;
		chdata_1.setAdc(adc_ch1, ch1_datatctp);
		chdata_3.setAdc(adc_ch3, ch3_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 2, 4, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);
	      chdata_3.setAdc(ch3_dataadc, ch3_datatctp);

	      ////////////// 2-DOF Double Ch-23 //////////////	      
	      for(uint32_t adc_ch23=0; adc_ch23<=2046; adc_ch23++){
		uint16_t adc_ch2 = TMath::Nint(adc_ch23/2);
		uint16_t adc_ch3 = adc_ch23 - adc_ch2;
		uint32_t totADC = adc_ch2 + adc_ch3;
		chdata_2.setAdc(adc_ch2, ch2_datatctp);
		chdata_3.setAdc(adc_ch3, ch3_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 2, 5, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_2.setAdc(ch2_dataadc, ch2_datatctp);
	      chdata_3.setAdc(ch3_dataadc, ch3_datatctp);
	      //============================================================================================
	      

	      //============================================================================================
	      // Emulation of 3 free parameters (3-DoF)
	      // In this order {"012", "013", "023", "123"};
	      //============================================================================================
	      ////////////// 3-DOF Triple Ch-012 //////////////	      
	      for(uint32_t adc_ch012=0; adc_ch012<=3069; adc_ch012++){
		uint16_t adc_ch0 = TMath::Nint(adc_ch012/3);
		uint16_t adc_ch1 = adc_ch0;
		uint16_t diff = adc_ch012 - (adc_ch0+adc_ch1);
		uint16_t adc_ch2 = (diff>1023)?(1023):diff;
		if(diff>1023){
		  diff = adc_ch012 - (adc_ch0+adc_ch2);
		  adc_ch1 = (diff>1023)?1023:diff;
		}
		uint32_t totADC = adc_ch0 + adc_ch1 + adc_ch2;
		chdata_0.setAdc(adc_ch0, ch0_datatctp);
		chdata_1.setAdc(adc_ch1, ch1_datatctp);
		chdata_2.setAdc(adc_ch2, ch2_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 3, 0, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);
	      chdata_2.setAdc(ch2_dataadc, ch2_datatctp);

	      ////////////// 3-DOF Triple Ch-013 //////////////	      
	      for(uint32_t adc_ch013=0; adc_ch013<=3069; adc_ch013++){
		uint16_t adc_ch0 = TMath::Nint(adc_ch013/3);
		uint16_t adc_ch1 = adc_ch0;
		uint16_t diff = adc_ch013 - (adc_ch0+adc_ch1);
		uint16_t adc_ch3 = (diff>1023)?(1023):diff;
		if(diff>1023){
		  diff = adc_ch013 - (adc_ch0+adc_ch3);
		  adc_ch1 = (diff>1023)?1023:diff;
		}
		uint32_t totADC = adc_ch0 + adc_ch1 + adc_ch3;
		chdata_0.setAdc(adc_ch0, ch0_datatctp);
		chdata_1.setAdc(adc_ch1, ch1_datatctp);
		chdata_3.setAdc(adc_ch3, ch3_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 3, 1, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);
	      chdata_3.setAdc(ch3_dataadc, ch3_datatctp);
	      
	      ////////////// 3-DOF Triple Ch-023 //////////////	      
	      for(uint32_t adc_ch023=0; adc_ch023<=3069; adc_ch023++){
		uint16_t adc_ch0 = TMath::Nint(adc_ch023/3);
		uint16_t adc_ch2 = adc_ch0;
		uint16_t diff = adc_ch023 - (adc_ch0+adc_ch2);
		uint16_t adc_ch3 = (diff>1023)?(1023):diff;
		if(diff>1023){
		  diff = adc_ch023 - (adc_ch0+adc_ch3);
		  adc_ch2 = (diff>1023)?1023:diff;
		}
		uint32_t totADC = adc_ch0 + adc_ch2 + adc_ch3;
		chdata_0.setAdc(adc_ch0, ch0_datatctp);
		chdata_2.setAdc(adc_ch2, ch2_datatctp);
		chdata_3.setAdc(adc_ch3, ch3_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 3, 2, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);
	      chdata_2.setAdc(ch2_dataadc, ch2_datatctp);
	      chdata_3.setAdc(ch3_dataadc, ch3_datatctp);

	      ////////////// 3-DOF Triple Ch-123 //////////////	      
	      for(uint32_t adc_ch123=0; adc_ch123<=3069; adc_ch123++){
		uint16_t adc_ch1 = TMath::Nint(adc_ch123/3);
		uint16_t adc_ch2 = adc_ch1;
		uint16_t diff = adc_ch123 - (adc_ch1+adc_ch2);
		uint16_t adc_ch3 = (diff>1023)?(1023):diff;
		if(diff>1023){
		  diff = adc_ch123 - (adc_ch1+adc_ch3);
		  adc_ch2 = (diff>1023)?1023:diff;
		}
		uint32_t totADC = adc_ch1 + adc_ch2 + adc_ch3;
		chdata_1.setAdc(adc_ch1, ch1_datatctp);
		chdata_2.setAdc(adc_ch2, ch2_datatctp);
		chdata_3.setAdc(adc_ch3, ch3_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 3, 3, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);
	      chdata_2.setAdc(ch2_dataadc, ch2_datatctp);
	      chdata_3.setAdc(ch3_dataadc, ch3_datatctp);

	      //============================================================================================

	      
	      //============================================================================================
	      // Emulation of 4 free parameters (4-DoF)
	      //============================================================================================
	      for(uint32_t adc_ch0123=0; adc_ch0123<=4092; adc_ch0123++){
	      	uint16_t adc_ch0 = TMath::Nint(adc_ch0123/4);
	      	uint16_t adc_ch1 = adc_ch0;
	      	uint16_t adc_ch2 = adc_ch0;
		uint16_t diff = adc_ch0123 - (adc_ch0+adc_ch1+adc_ch2);
	      	uint16_t adc_ch3 = (diff>1023)?(1023):diff;
		if(diff>1023){
		  diff = adc_ch0123 - (adc_ch0+adc_ch1+adc_ch3);
		  adc_ch2 = (diff>1023)?1023:diff;
		  if(diff>1023){
		    diff = adc_ch0123 - (adc_ch0+adc_ch2+adc_ch3);
		    adc_ch1 = (diff>1023)?1023:diff;
		  }
		}
		uint32_t totADC = adc_ch0 + adc_ch1 + adc_ch2 + adc_ch3;
		//std::cerr<<"adc_ch0123: " << adc_ch0123 << ", adc_ch0: "<<adc_ch0 << ", adc_ch1: "<<adc_ch1<< ", adc_ch2: "<<adc_ch2 << ", adc_ch3: "<<adc_ch3 << std::endl;
	      	chdata_0.setAdc(adc_ch0, ch0_datatctp);
	      	chdata_1.setAdc(adc_ch1, ch1_datatctp);
	      	chdata_2.setAdc(adc_ch2, ch2_datatctp);
	      	chdata_3.setAdc(adc_ch3, ch3_datatctp); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
	      	uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
	      	FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 4, 0, totADC, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);
	      chdata_2.setAdc(ch2_dataadc, ch2_datatctp);
	      chdata_3.setAdc(ch3_dataadc, ch3_datatctp);
	      //============================================================================================
	      
	      
	      //============================================================================================
	      //Emulation of 1 free parameter (1-DoF) for TOT
	      //============================================================================================
	      ////////////// 1-DOF Single Ch-0 //////////////
	      chdata_0.setZero();
	      //econtEmul.setVerbose();
	      for(uint16_t tot_ch0=0; tot_ch0<=4095; tot_ch0++){
		chdata_0.setTot(tot_ch0, 0x3);
		if(tot_ch0==200) { std::cout << "==itc========" << itc << std::endl; chdata_0.print(); hrocdata_0.print();}
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
		if(itc==1 and tot_ch0==200)
		  rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata, event);
		else
		  rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
		if(itc==1 and tot_ch0==200 and moduleId==256) modTcdata.second.print();
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 5, 0, tot_ch0, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setZero();
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);
	      //econtEmul.setVerbose(false);
	      
	      ////////////// 1-DOF Single Ch-1 //////////////
	      chdata_1.setZero();
	      for(uint16_t tot_ch1=0; tot_ch1<=4095; tot_ch1++){
		chdata_1.setTot(tot_ch1, 0x3);
		//if(tot_ch1==100) { std::cout << "==itc========" << itc << std::endl; chdata_1.print(); hrocdata_1.print();}
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 5, 1, tot_ch1, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_1.setZero();
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);

	      ////////////// 1-DOF Single Ch-2 //////////////
	      chdata_2.setZero();
	      for(uint16_t tot_ch2=0; tot_ch2<=4095; tot_ch2++){
		chdata_2.setTot(tot_ch2, 0x3);
		//if(tot_ch2==100) { std::cout << "==itc========" << itc << std::endl; chdata_2.print(); hrocdata_2.print();}
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 5, 2, tot_ch2, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_2.setZero();
	      chdata_2.setAdc(ch2_dataadc, ch2_datatctp);

	      ////////////// 1-DOF Single Ch-2 //////////////
	      chdata_3.setZero();
	      for(uint16_t tot_ch3=0; tot_ch3<=4095; tot_ch3++){
		chdata_3.setTot(tot_ch3, 0x3);
		//if(tot_ch3==100) { std::cout << "==itc========" << itc << std::endl; chdata_3.print(); hrocdata_3.print();}
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 5, 3, tot_ch3, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_3.setZero();
	      chdata_3.setAdc(ch3_dataadc, ch3_datatctp);
	      
	      //============================================================================================
	      // Emulation of 2 free parameters (2-DoF) for TOT
	      // In this order {"01", "02", "03", "12", "13", "23"};
	      //============================================================================================
	      ////////////// 2-DOF Double Ch-01 //////////////
	      chdata_0.setZero();
	      chdata_1.setZero();
	      for(uint32_t tot_ch01=0; tot_ch01<=8190; tot_ch01++){
		uint16_t tot_ch0 = TMath::Nint(tot_ch01/2);
		uint16_t tot_ch1 = tot_ch01 - tot_ch0;
		uint32_t totTOT = tot_ch0 + tot_ch1;
		chdata_0.setTot(tot_ch0, 0x3);
		chdata_1.setTot(tot_ch1, 0x3); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
		uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
		FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 6, 0, totTOT, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setZero();
	      chdata_1.setZero();
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);
	      
	      //============================================================================================
	      // Emulation of 4 free parameters (4-DoF) for TOT
	      //============================================================================================
	      chdata_0.setZero(); chdata_1.setZero(); chdata_2.setZero(); chdata_3.setZero();
	      for(uint32_t tot_ch0123=0; tot_ch0123<=16380; tot_ch0123++){
	      	uint16_t tot_ch0 = TMath::Nint(tot_ch0123/4);
	      	uint16_t tot_ch1 = tot_ch0;
	      	uint16_t tot_ch2 = tot_ch0;
		uint16_t diff = tot_ch0123 - (tot_ch0+tot_ch1+tot_ch2);
	      	uint16_t tot_ch3 = (diff>4095)?(4095):diff;
		if(diff>4095){
		  diff = tot_ch0123 - (tot_ch0+tot_ch1+tot_ch3);
		  tot_ch2 = (diff>4095)?4095:diff;
		  if(diff>4095){
		    diff = tot_ch0123 - (tot_ch0+tot_ch2+tot_ch3);
		    tot_ch1 = (diff>4095)?4095:diff;
		  }
		}
		uint32_t totTOT = tot_ch0 + tot_ch1 + tot_ch2 + tot_ch3;
		//std::cerr<<"tot_ch0123: " << tot_ch0123 << ", tot_ch0: "<<tot_ch0 << ", tot_ch1: "<<tot_ch1<< ", tot_ch2: "<<tot_ch2 << ", tot_ch3: "<<tot_ch3 << std::endl;
	      	chdata_0.setTot(tot_ch0, 0x3);
	      	chdata_1.setTot(tot_ch1, 0x3);
	      	chdata_2.setTot(tot_ch2, 0x3);
	      	chdata_3.setTot(tot_ch3, 0x3); 
	        std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	        //================================================
	        //HGCROC emulation for a given module
	        //================================================
	        rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	        //================================================
	        modarray.clear() ; modarray[event].push_back(modTcdata);	  
	        moddata.clear(); for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	        //================================================
	        //ECONT emulation for a given module
	        //================================================
	        econtEmul.Emulate(false, event, moduleId, moddata);
	        TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	        //================================================
	        if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	        TcRawdata.second.setBX(emul_bx_4b);
	      	uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
	      	FillHistogram(dir_diff, relayNumber, event, econt_slink_bx, itc, 8, 0, totTOT, tc_address, tc_energy_data, tc_energy_emul);
	      }
	      chdata_0.setZero(); chdata_1.setZero(); chdata_2.setZero(); chdata_3.setZero();
	      chdata_0.setAdc(ch0_dataadc, ch0_datatctp);
	      chdata_1.setAdc(ch1_dataadc, ch1_datatctp);
	      chdata_2.setAdc(ch2_dataadc, ch2_datatctp);
	      chdata_3.setAdc(ch3_dataadc, ch3_datatctp);
	      //============================================================================================

	      delete []rocid;
	      delete []rocpin;
	    }//loop over data TCs	  
	  }//event,ilink and iecond condition
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

  void Plot(TDirectory*& dir_diff);

  fout->cd();
  Plot(dir_diff);

  fout->cd();
  dir_diff->Write();
  fout->Close();
  delete fout;
  
  return true;
}

void Plot(TDirectory*& dir_diff){

  TList *list = (TList *)dir_diff->GetList();
  const int nofTCs = 6;
  int TC[nofTCs] = {0, 3, 6, 11, 20, 31};
  const int nofSCh = 4;
  const int nofDCh = 6;
  const char* dpair[nofDCh] = {"01", "02", "03", "12", "13", "23"};
  const int nofTCh = 4;
  const char* ttriad[nofTCh] = {"012", "013", "023", "123"};
  
  for(int itc=0;itc<nofTCs;itc++){
    
    TCanvas* c1 = new TCanvas(Form("c1_%d_1dof",TC[itc]),Form("c1_%d_1dof",TC[itc]),1600,600);
    c1->Divide(nofSCh,1);
    for(int i1dof=0;i1dof<nofSCh;i1dof++){
      c1->cd(i1dof+1);
      ((TH1D *) list->FindObject(Form("hADCScan_TC%d_SCh%d",itc,i1dof)))->Draw();
    }
    c1->Write();
    
    TCanvas* c2 = new TCanvas(Form("c2_%d_2dof",TC[itc]),Form("c2_%d_2dof",TC[itc]),1600,600);
    c2->Divide(nofDCh,1);
    for(int i2dof=0;i2dof<nofDCh;i2dof++){
      c2->cd(i2dof+1);
      ((TH1D *) list->FindObject(Form("hADCScan_TC%d_DCh%d",itc,i2dof)))->Draw();
    }
    c2->Write();
    
    TCanvas* c3 = new TCanvas(Form("c3_%d_3dof",TC[itc]),Form("c3_%d_3dof",TC[itc]),1600,600);
    c3->Divide(nofTCh,1);
    for(int i3dof=0;i3dof<nofTCh;i3dof++){
      c3->cd(i3dof+1);
      ((TH1D *) list->FindObject(Form("hADCScan_TC%d_TCh%d",itc,i3dof)))->Draw();
    }
    c3->Write();
    
    TCanvas* c4 = new TCanvas(Form("c4_%d_4dof",TC[itc]),Form("c4_%d_4dof",TC[itc]),900,900);
    ((TH1D *) list->FindObject(Form("hADCScan_TC%d_QuadCh",itc)))->Draw();
    c4->Write();
    
    TCanvas* c5 = new TCanvas(Form("c5_%d_TOT_1dof",TC[itc]),Form("c5_%d_TOT_1dof",TC[itc]),900,900);
    ((TH1D *) list->FindObject(Form("hTOTScan_TC%d_1dof",itc)))->Draw();
    c5->Write();
    
    TCanvas* c6 = new TCanvas(Form("c6_%d_TOT_2dof",TC[itc]),Form("c6_%d_TOT_2dof",TC[itc]),900,900);
    ((TH1D *) list->FindObject(Form("hTOTScan_TC%d_2dof",itc)))->Draw();
    c6->Write();

    TCanvas* c8 = new TCanvas(Form("c8_%d_TOT_4dof",TC[itc]),Form("c8_%d_TOT_4dof",TC[itc]),900,900);
    ((TH1D *) list->FindObject(Form("hTOTScan_TC%d_4dof",itc)))->Draw();
    c8->Write();

  }
}

void BookHistograms(TDirectory*& dir_diff, uint32_t relay){

  Color_t color = kYellow+1;
  
  const int nofTCs = 6;
  int TC[nofTCs] = {0, 3, 6, 11, 20, 31};
  const int nofSCh = 4;
  const int nofDCh = 6;
  const char* dpair[nofDCh] = {"01", "02", "03", "12", "13", "23"};
  const int nofTCh = 4;
  const char* ttriad[nofTCh] = {"012", "013", "023", "123"};
  
  TH1D *hADCscan1dof[nofTCs][nofSCh], *hADCscan2dof[nofTCs][nofDCh], *hADCscan3dof[nofTCs][nofTCh];
  TH1D *hADCscan4dof[nofTCs];
  TH1D *hTOTscan1dof[nofTCs], *hTOTscan2dof[nofTCs];
  TH1D *hTOTscan4dof[nofTCs];
  
  for(int itc=0;itc<nofTCs;itc++){
    for(int i1dof=0;i1dof<nofSCh;i1dof++){
      hADCscan1dof[itc][i1dof] = new TH1D(Form("hADCScan_TC%d_SCh%d",itc,i1dof), Form("ADC scan TC-%d with Ch-%d input for Relay: %u",TC[itc],i1dof,relay), 1026, -1.5, 1024.5);
      hADCscan1dof[itc][i1dof]->GetXaxis()->SetTitle("input ADC");
      hADCscan1dof[itc][i1dof]->GetYaxis()->SetTitle("Entries");
      hADCscan1dof[itc][i1dof]->SetFillColor(color);
      hADCscan1dof[itc][i1dof]->SetDirectory(dir_diff);
    }
    for(int i2dof=0;i2dof<nofDCh;i2dof++){
      hADCscan2dof[itc][i2dof] = new TH1D(Form("hADCScan_TC%d_DCh%d",itc,i2dof), Form("ADC scan TC-%d Ch-%s for Relay: %u",TC[itc],dpair[i2dof],relay), 2049, -1.5, 2047.5);
      hADCscan2dof[itc][i2dof]->GetXaxis()->SetTitle("input ADC");
      hADCscan2dof[itc][i2dof]->GetYaxis()->SetTitle("Entries");
      hADCscan2dof[itc][i2dof]->SetFillColor(color);
      hADCscan2dof[itc][i2dof]->SetDirectory(dir_diff);
    }
    for(int i3dof=0;i3dof<nofTCh;i3dof++){
      hADCscan3dof[itc][i3dof] = new TH1D(Form("hADCScan_TC%d_TCh%d",itc,i3dof), Form("ADC scan TC-%d Ch-%s for Relay: %u",TC[itc],ttriad[i3dof],relay), 3072, -1.5, 3070.5);
      hADCscan3dof[itc][i3dof]->GetXaxis()->SetTitle("input ADC");
      hADCscan3dof[itc][i3dof]->GetYaxis()->SetTitle("Entries");
      hADCscan3dof[itc][i3dof]->SetFillColor(color);
      hADCscan3dof[itc][i3dof]->SetDirectory(dir_diff);
    }    
    hADCscan4dof[itc] = new TH1D(Form("hADCScan_TC%d_QuadCh",itc), Form("ADC scan TC-%d Ch-0123 for Relay: %u",TC[itc],relay), 4095, -1.5, 4093.5);
    hADCscan4dof[itc]->GetXaxis()->SetTitle("input ADC");
    hADCscan4dof[itc]->GetYaxis()->SetTitle("Entries");
    hADCscan4dof[itc]->SetFillColor(color);
    hADCscan4dof[itc]->SetDirectory(dir_diff);


    hTOTscan1dof[itc] = new TH1D(Form("hTOTScan_TC%d_1dof",itc), Form("TOT scan TC-%d Ch-0 for Relay: %u",TC[itc],relay), 4098, -1.5, 4096.5);
    hTOTscan1dof[itc]->GetXaxis()->SetTitle("input TOT");
    hTOTscan1dof[itc]->GetYaxis()->SetTitle("Entries");
    hTOTscan1dof[itc]->SetFillColor(color);
    hTOTscan1dof[itc]->SetDirectory(dir_diff);
    
    hTOTscan2dof[itc] = new TH1D(Form("hTOTScan_TC%d_2dof",itc), Form("TOT scan TC-%d Ch-01 for Relay: %u",TC[itc],relay), 8193, -1.5, 8191.5);
    hTOTscan2dof[itc]->GetXaxis()->SetTitle("input TOT");
    hTOTscan2dof[itc]->GetYaxis()->SetTitle("Entries");
    hTOTscan2dof[itc]->SetFillColor(color);
    hTOTscan2dof[itc]->SetDirectory(dir_diff);
    
    hTOTscan4dof[itc] = new TH1D(Form("hTOTScan_TC%d_4dof",itc), Form("TOT scan TC-%d Ch-0123 for Relay: %u",TC[itc],relay), 16383, -1.5, 16381.5);
    hTOTscan4dof[itc]->GetXaxis()->SetTitle("input TOT");
    hTOTscan4dof[itc]->GetYaxis()->SetTitle("Entries");
    hTOTscan4dof[itc]->SetFillColor(color);
    hTOTscan4dof[itc]->SetDirectory(dir_diff);

  }//itc loop
  
}

void FillHistogram(TDirectory*& dir_diff, uint32_t relay, uint64_t event, uint16_t bx, uint32_t itc, uint32_t nofpara, uint32_t hist_index, uint32_t input_adc, uint32_t tc_address, uint32_t tc_energy_data, uint32_t tc_energy_emul)
{
  TList *list = (TList *)dir_diff->GetList();
  uint32_t diff = tc_energy_data - tc_energy_emul;
  if(diff!=0 and nofpara==1) ((TH1D *) list->FindObject(Form("hADCScan_TC%d_SCh%u",itc,hist_index)))->Fill( input_adc );
  if(diff!=0 and nofpara==2) ((TH1D *) list->FindObject(Form("hADCScan_TC%d_DCh%u",itc,hist_index)))->Fill( input_adc );
  if(diff!=0 and nofpara==3) ((TH1D *) list->FindObject(Form("hADCScan_TC%d_TCh%u",itc,hist_index)))->Fill( input_adc );
  if(diff!=0 and nofpara==4) ((TH1D *) list->FindObject(Form("hADCScan_TC%d_QuadCh",itc)))->Fill( input_adc );
  if(diff!=0 and nofpara==5) ((TH1D *) list->FindObject(Form("hTOTScan_TC%d_1dof",itc)))->Fill( input_adc );
  if(diff!=0 and nofpara==6) ((TH1D *) list->FindObject(Form("hTOTScan_TC%d_2dof",itc)))->Fill( input_adc );
  if(diff!=0 and nofpara==8) ((TH1D *) list->FindObject(Form("hTOTScan_TC%d_4dof",itc)))->Fill( input_adc );
  
}
